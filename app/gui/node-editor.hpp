// Copyright (c) 2020 INRA Distributed under the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ORG_VLEPROJECT_IRRITATOR_APP_NODE_EDITOR_2020
#define ORG_VLEPROJECT_IRRITATOR_APP_NODE_EDITOR_2020

#include <irritator/core.hpp>
#include <irritator/external_source.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <thread>
#include <variant>
#include <vector>

#include "imnodes.hpp"
#include <imgui.h>

namespace irt {

inline const char*
status_string(const status s) noexcept
{
    static const char* str[] = {
        "success",
        "unknown_dynamics",
        "block_allocator_bad_capacity",
        "block_allocator_not_enough_memory",
        "head_allocator_bad_capacity",
        "head_allocator_not_enough_memory",
        "simulation_not_enough_model",
        "simulation_not_enough_memory_message_list_allocator",
        "simulation_not_enough_memory_input_port_list_allocator",
        "simulation_not_enough_memory_output_port_list_allocator",
        "data_array_init_capacity_error",
        "data_array_not_enough_memory",
        "data_array_archive_init_capacity_error",
        "data_array_archive_not_enough_memory",
        "array_init_capacity_zero",
        "array_init_capacity_too_big",
        "array_init_not_enough_memory",
        "vector_init_capacity_zero",
        "vector_init_capacity_too_big",
        "vector_init_not_enough_memory",
        "dynamics_unknown_id",
        "dynamics_unknown_port_id",
        "dynamics_not_enough_memory",
        "model_connect_output_port_unknown",
        "model_connect_input_port_unknown",
        "model_connect_already_exist",
        "model_connect_bad_dynamics",
        "model_queue_bad_ta",
        "model_queue_empty_allocator",
        "model_queue_full",
        "model_dynamic_queue_source_is_null",
        "model_dynamic_queue_empty_allocator",
        "model_dynamic_queue_full",
        "model_priority_queue_source_is_null",
        "model_priority_queue_empty_allocator",
        "model_priority_queue_full",
        "model_integrator_dq_error",
        "model_integrator_X_error",
        "model_integrator_internal_error",
        "model_integrator_output_error",
        "model_integrator_running_without_x_dot",
        "model_integrator_ta_with_bad_x_dot",
        "model_generator_null_ta_source",
        "model_generator_empty_ta_source",
        "model_generator_null_value_source",
        "model_generator_empty_value_source",
        "model_quantifier_bad_quantum_parameter",
        "model_quantifier_bad_archive_length_parameter",
        "model_quantifier_shifting_value_neg",
        "model_quantifier_shifting_value_less_1",
        "model_time_func_bad_init_message",
        "model_flow_bad_samplerate",
        "model_flow_bad_data",
        "gui_not_enough_memory",
        "io_not_enough_memory",
        "io_file_format_error",
        "io_file_format_model_error",
        "io_file_format_model_number_error",
        "io_file_format_model_unknown",
        "io_file_format_dynamics_unknown",
        "io_file_format_dynamics_limit_reach",
        "io_file_format_dynamics_init_error"
    };

    static_assert(std::size(str) == status_size());

    return str[static_cast<int>(s)];
}

enum class editor_id : u64;
enum class cluster_id : u64;

using child_id = std::variant<model_id, cluster_id>;

enum class editor_status
{
    editing,
    initializing,
    running_debug,
    running_thread,
    running_thread_need_join
};

static inline constexpr int not_found = -1;

struct top_cluster
{
    std::vector<std::pair<child_id, int>> children;
    int next_node_id = 0;

    static inline constexpr int not_found = -1;

    status init(size_t models) noexcept
    {
        try {
            children.reserve(models);
        } catch (const std::bad_alloc&) {
            std::vector<std::pair<child_id, int>>().swap(children);
            irt_bad_return(status::gui_not_enough_memory);
        }

        return status::success;
    }

    int get_index(const child_id id) const noexcept
    {
        for (int i = 0, e = length(children); i != e; ++i)
            if (children[i].first == id)
                return i;

        return not_found;
    }

    int get_index(const int node) const noexcept
    {
        for (int i = 0, e = length(children); i != e; ++i)
            if (children[i].second == node)
                return i;

        return not_found;
    }

    void clear() noexcept
    {
        children.clear();
    }

    void pop(const int index) noexcept
    {
        std::swap(children[index], children.back());
        children.pop_back();
    }

    int emplace_back(const child_id id)
    {
        int ret = next_node_id++;

        children.emplace_back(id, ret);

        return ret;
    }
};

inline int
make_input_node_id(const irt::model_id mdl, const int port) noexcept
{
    irt_assert(port >= 0 && port < 8);

    irt::u32 index = irt::get_index(mdl);
    irt_assert(index < 268435456u);

    irt::u32 port_index = static_cast<irt::u32>(port) << 28u;
    index |= port_index;

    return static_cast<int>(index);
}

inline int
make_output_node_id(const irt::model_id mdl, const int port) noexcept
{
    irt_assert(port >= 0 && port < 8);

    irt::u32 index = irt::get_index(mdl);
    irt_assert(index < 268435456u);

    irt::u32 port_index = static_cast<irt::u32>(8u + port) << 28u;

    index |= port_index;

    return static_cast<int>(index);
}

inline std::pair<irt::u32, irt::u32>
get_model_input_port(const int node_id) noexcept
{
    const irt::u32 real_node_id = static_cast<irt::u32>(node_id);

    irt::u32 port = real_node_id >> 28u;
    irt_assert(port < 8u);

    constexpr irt::u32 mask = ~(15u << 28u);
    irt::u32 index = real_node_id & mask;

    return std::make_pair(index, port);
}

inline std::pair<irt::u32, irt::u32>
get_model_output_port(const int node_id) noexcept
{
    const irt::u32 real_node_id = static_cast<irt::u32>(node_id);

    irt::u32 port = real_node_id >> 28u;

    irt_assert(port >= 8u && port < 16u);
    port -= 8u;
    irt_assert(port < 8u);

    constexpr irt::u32 mask = ~(15u << 28u);

    irt::u32 index = real_node_id & mask;

    return std::make_pair(index, port);
}

struct cluster
{
    cluster() = default;

    small_string<16> name;
    std::vector<child_id> children;
    std::vector<int> input_ports;
    std::vector<int> output_ports;

    int get(const child_id id) const noexcept
    {
        auto it = std::find(std::begin(children), std::end(children), id);
        if (it == std::end(children))
            return not_found;

        return static_cast<int>(std::distance(std::begin(children), it));
    }
};

struct window_logger
{
    ImGuiTextBuffer buffer;
    ImGuiTextFilter filter;
    ImVector<int> line_offsets;

    bool auto_scroll = true;
    bool scroll_to_bottom = false;
    window_logger() = default;
    void clear() noexcept;

    void log(const int level, const char* fmt, ...) IM_FMTARGS(3);
    void log(const int level, const char* fmt, va_list args) IM_FMTLIST(3);
    void show(bool* is_show);
};

static inline window_logger log_w;

struct editor;

enum class plot_output_id : u64;
enum class file_output_id : u64;
enum class file_discrete_output_id : u64;

struct plot_output
{
    plot_output() = default;

    plot_output(std::string_view name_)
      : name(name_)
    {}

    void operator()(const irt::observer& obs,
                    const irt::dynamics_type /*type*/,
                    const irt::time tl,
                    const irt::time t,
                    const irt::observer::status s);

    editor* ed = nullptr;
    std::vector<float> xs;
    std::vector<float> ys;
    small_string<24u> name;
    double tl = 0.0;
    double time_step = 0.01;
};

struct file_output
{
    file_output() = default;

    file_output(std::string_view name_)
      : name(name_)
    {}

    void operator()(const irt::observer& obs,
                    const irt::dynamics_type type,
                    const irt::time tl,
                    const irt::time t,
                    const irt::observer::status s);

    editor* ed = nullptr;
    std::ofstream ofs;
    small_string<24u> name;
};

struct file_discrete_output
{
    file_discrete_output() = default;

    file_discrete_output(std::string_view name_)
      : name(name_)
    {}

    void operator()(const irt::observer& obs,
                    const irt::dynamics_type type,
                    const irt::time tl,
                    const irt::time t,
                    const irt::observer::status s);

    editor* ed = nullptr;
    std::ofstream ofs;
    small_string<24u> name;
    double tl = 0.0;
    double time_step = 0.01;
};

using observation_output = std::variant<std::monostate,
                                        plot_output_id,
                                        file_output_id,
                                        file_discrete_output_id>;

struct editor
{
    small_string<16> name;
    std::filesystem::path path;
    imnodes::EditorContext* context = nullptr;
    bool initialized = false;
    bool show = true;

    simulation sim;

    double simulation_begin = 0.0;
    double simulation_end = 10.0;
    double simulation_current = 10.0;
    double simulation_next_time = 0.0;
    long simulation_bag_id = 0;

    double simulation_during_date;
    int simulation_during_bag;

    std::thread simulation_thread;
    editor_status st = editor_status::editing;
    status sim_st = status::success;

    bool simulation_show_value = false;
    bool stop = false;

    data_array<plot_output, plot_output_id> plot_outs;
    data_array<file_output, file_output_id> file_outs;
    data_array<file_discrete_output, file_discrete_output_id>
      file_discrete_outs;
    std::vector<observation_output> observation_outputs;

    template<typename Function, typename... Args>
    constexpr void observation_dispatch(const u32 index,
                                        Function&& f,
                                        Args... args) noexcept
    {
        switch (observation_outputs[index].index()) {
        case 1:
            f(plot_outs,
              std::get<plot_output_id>(observation_outputs[index]),
              args...);
            break;

        case 2:
            f(file_outs,
              std::get<file_output_id>(observation_outputs[index]),
              args...);
            break;

        case 3:
            f(file_discrete_outs,
              std::get<file_discrete_output_id>(observation_outputs[index]),
              args...);
            break;

        default:
            break;
        }
    }

    void observation_outputs_free(const u32 index) noexcept
    {
        observation_dispatch(
          index, [](auto& outs, auto out_id) { outs.free(out_id); });

        observation_outputs[index] = std::monostate{};
    }

    std::filesystem::path observation_directory;

    data_array<cluster, cluster_id> clusters;
    std::vector<cluster_id> clusters_mapper; /* group per cluster_id */
    std::vector<cluster_id> models_mapper;   /* group per model_id */

    std::vector<bool> models_make_transition;

    ImVector<ImVec2> positions;
    ImVector<ImVec2> displacements;

    bool use_real_time;
    bool starting = true;
    double synchronize_timestep;

    top_cluster top;

    std::string tooltip;

    bool show_load_file_dialog = false;
    bool show_save_file_dialog = false;
    bool show_select_directory_dialog = false;
    bool show_settings = false;

    struct settings_manager
    {
        int kernel_model_cache = 1024;
        int kernel_message_cache = 32768;
        int gui_node_cache = 1024;
        ImVec4 gui_model_color{ .27f, .27f, .54f, 1.f };
        ImVec4 gui_model_transition_color{ .27f, .54f, .54f, 1.f };
        ImVec4 gui_cluster_color{ .27f, .54f, .27f, 1.f };

        ImU32 gui_hovered_model_color;
        ImU32 gui_selected_model_color;
        ImU32 gui_hovered_model_transition_color;
        ImU32 gui_selected_model_transition_color;
        ImU32 gui_hovered_cluster_color;
        ImU32 gui_selected_cluster_color;

        int automatic_layout_iteration_limit = 200;
        float automatic_layout_x_distance = 350.f;
        float automatic_layout_y_distance = 350.f;
        float grid_layout_x_distance = 250.f;
        float grid_layout_y_distance = 250.f;

        bool show_dynamics_inputs_in_editor = false;

        void compute_colors() noexcept;
        void show(bool* is_open);

    } settings;

    status initialize(u32 id) noexcept;
    void clear() noexcept;

    void group(const ImVector<int>& nodes) noexcept;
    void ungroup(const int node) noexcept;
    void free_group(cluster& group) noexcept;
    void free_children(const ImVector<int>& nodes) noexcept;
    status copy(const ImVector<int>& nodes) noexcept;

    void compute_grid_layout() noexcept;
    void compute_automatic_layout() noexcept;

    bool is_in_hierarchy(const cluster& group,
                         const cluster_id group_to_search) const noexcept;
    cluster_id ancestor(const child_id child) const noexcept;
    int get_top_group_ref(const child_id child) const noexcept;

    cluster_id parent(cluster_id child) const noexcept
    {
        return clusters_mapper[get_index(child)];
    }

    cluster_id parent(model_id child) const noexcept
    {
        return models_mapper[get_index(child)];
    }

    void parent(const cluster_id child, const cluster_id parent) noexcept
    {
        clusters_mapper[get_index(child)] = parent;
    }

    void parent(const model_id child, const cluster_id parent) noexcept
    {
        models_mapper[get_index(child)] = parent;
    }

    struct gport
    {
        gport() noexcept = default;

        gport(irt::model* model_, const int port_index_) noexcept
          : model(model_)
          , port_index(port_index_)
        {}

        irt::model* model = nullptr;
        int port_index = 0;
    };

    gport get_in(const int index) noexcept
    {
        const auto model_index_port = get_model_input_port(index);
        auto* mdl = sim.models.try_to_get(model_index_port.first);

        return { mdl, static_cast<int>(model_index_port.second) };
    }

    gport get_out(const int index) noexcept
    {
        const auto model_index_port = get_model_output_port(index);
        auto* mdl = sim.models.try_to_get(model_index_port.first);

        return { mdl, static_cast<int>(model_index_port.second) };
    }

    status add_lotka_volterra() noexcept;
    status add_izhikevitch() noexcept;

    void show_connections() noexcept;
    void show_model_dynamics(model& mdl) noexcept;
    void show_model_cluster(cluster& mdl) noexcept;
    void show_top() noexcept;

    bool show_editor() noexcept;
};

void
show_simulation_box(editor& ed, bool* show_simulation);

struct application
{
    data_array<editor, editor_id> editors;

    struct settings_manager
    {
        settings_manager() noexcept;

        std::filesystem::path home_dir;
        std::filesystem::path executable_dir;
        std::vector<std::string> libraries_dir;

        void show(bool* is_open);
    } settings;

    external_source srcs;
    void show_sources(bool* is_show);
    void show_menu_sources(const char* title, source& src);

    bool show_log = true;
    bool show_simulation = true;
    bool show_demo = false;
    bool show_plot = true;
    bool show_settings = false;
    bool show_sources_window = false;

    editor* alloc_editor();
    void free_editor(editor& ed);
};

static inline application app;

editor*
make_combo_editor_name(application& app, editor_id& current) noexcept;

} // namespace irt

#endif
