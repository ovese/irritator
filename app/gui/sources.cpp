// Copyright (c) 2020 INRA Distributed under the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "gui.hpp"
#include "node-editor.hpp"

#include <fmt/format.h>

#include <future>
#include <random>
#include <thread>

#include <cinttypes>

namespace irt {

template<size_t N, typename... Args>
void
format(small_string<N>& str, const char* fmt, const Args&... args)
{
    auto ret = fmt::format_to_n(str.begin(), N - 1, fmt, args...);
    str.size(ret.size);
}

// static void
// show_random(irt::random_source& src)
// {
//     static int current_item = -1;
//     static u64 size = 1024 * 1024;
//     static bool show_file_dialog = false;

//     if (ImGui::CollapsingHeader("Random source")) {
//         ImGui::InputScalar("length", ImGuiDataType_U64, &size);

//         int old_current = current_item;
//         ImGui::Combo("Distribution",
//                      &current_item,
//                      irt::distribution_type_str,
//                      IM_ARRAYSIZE(irt::distribution_type_str));

//         const auto type = enum_cast<irt::distribution_type>(current_item);

//         switch (type) {
//         case distribution_type::uniform_int:
//             if (old_current != current_item) {
//                 src.a32 = 0;
//                 src.b32 = 100;
//             }
//             ImGui::InputInt("a", &src.a32);
//             ImGui::InputInt("b", &src.b32);
//             break;

//         case distribution_type::uniform_real:
//             if (old_current != current_item) {
//                 src.a = 0.0;
//                 src.b = 1.0;
//             }
//             ImGui::InputDouble("a", &src.a);
//             ImGui::InputDouble("b", &src.b);
//             break;

//         case distribution_type::bernouilli:
//             if (old_current != current_item) {
//                 src.p = 0.5;
//             }
//             ImGui::InputDouble("p", &src.p);
//             break;

//         case distribution_type::binomial:
//             if (old_current != current_item) {
//                 src.p = 0.5;
//                 src.t32 = 1;
//             }
//             ImGui::InputDouble("p", &src.p);
//             ImGui::InputInt("t", &src.t32);
//             break;

//         case distribution_type::negative_binomial:
//             if (old_current != current_item) {
//                 src.p = 0.5;
//                 src.t32 = 1;
//             }
//             ImGui::InputDouble("p", &src.p);
//             ImGui::InputInt("t", &src.k32);
//             break;

//         case distribution_type::geometric:
//             if (old_current != current_item) {
//                 src.p = 0.5;
//             }
//             ImGui::InputDouble("p", &src.p);
//             break;

//         case distribution_type::poisson:
//             if (old_current != current_item) {
//                 src.mean = 0.5;
//             }
//             ImGui::InputDouble("mean", &src.mean);
//             break;

//         case distribution_type::exponential:
//             if (old_current != current_item) {
//                 src.lambda = 1.0;
//             }
//             ImGui::InputDouble("lambda", &src.lambda);
//             break;

//         case distribution_type::gamma:
//             if (old_current != current_item) {
//                 src.alpha = 1.0;
//                 src.beta = 1.0;
//             }
//             ImGui::InputDouble("alpha", &src.alpha);
//             ImGui::InputDouble("beta", &src.beta);
//             break;

//         case distribution_type::weibull:
//             if (old_current != current_item) {
//                 src.a = 1.0;
//                 src.b = 1.0;
//             }
//             ImGui::InputDouble("a", &src.a);
//             ImGui::InputDouble("b", &src.b);
//             break;

//         case distribution_type::exterme_value:
//             if (old_current != current_item) {
//                 src.a = 1.0;
//                 src.b = 0.0;
//             }
//             ImGui::InputDouble("a", &src.a);
//             ImGui::InputDouble("b", &src.b);
//             break;

//         case distribution_type::normal:
//             if (old_current != current_item) {
//                 src.mean = 0.0;
//                 src.stddev = 1.0;
//             }
//             ImGui::InputDouble("mean", &src.mean);
//             ImGui::InputDouble("stddev", &src.stddev);
//             break;

//         case distribution_type::lognormal:
//             if (old_current != current_item) {
//                 src.m = 0.0;
//                 src.s = 1.0;
//             }
//             ImGui::InputDouble("m", &src.m);
//             ImGui::InputDouble("s", &src.s);
//             break;

//         case distribution_type::chi_squared:
//             if (old_current != current_item) {
//                 src.n = 1.0;
//             }
//             ImGui::InputDouble("n", &src.n);
//             break;

//         case distribution_type::cauchy:
//             if (old_current != current_item) {
//                 src.a = 1.0;
//                 src.b = 0.0;
//             }
//             ImGui::InputDouble("a", &src.a);
//             ImGui::InputDouble("b", &src.b);
//             break;

//         case distribution_type::fisher_f:
//             if (old_current != current_item) {
//                 src.m = 1.0;
//                 src.n = 1.0;
//             }
//             ImGui::InputDouble("m", &src.m);
//             ImGui::InputDouble("s", &src.n);
//             break;

//         case distribution_type::student_t:
//             if (old_current != current_item) {
//                 src.n = 1.0;
//             }
//             ImGui::InputDouble("n", &src.n);
//             break;
//         }

//         std::mt19937_64 dist(1024);

//         if (dm.is_running) {
//             ImGui::Text("File generation in progress %.2f", dm.status);

//             if (dm.status < 0.0 || dm.status >= 100.0) {
//                 dm.job.join();
//                 dm.status = 0.0;
//                 dm.is_running = false;
//                 dm.os.close();
//             }
//         } else if (current_item >= 0) {
//             if (ImGui::Button("Generate"))
//                 show_file_dialog = true;

//             if (show_file_dialog) {
//                 const char* title = "Select file path to save";
//                 const char8_t* filters[] = { u8".dat", nullptr };

//                 ImGui::OpenPopup(title);
//                 std::filesystem::path path;
//                 if (save_file_dialog(path, title, filters)) {
//                     show_file_dialog = false;

//                     log_w.log(5,
//                               "Save random generated file to %s\n",
//                               (const char*)path.u8string().c_str());

//                     if (dm.os = std::ofstream(path); dm.os.is_open())
//                         dm.start(dist,
//                                  size,
//                                  use_binary
//                                    ? irt::random_file_type::binary
//                                    : irt::source::random_file_type::text);
//                 }
//             }
//         }
//     }
// }

// static void
// size_in_bytes(const sources& src) noexcept
// {
//     constexpr sz K = 1024u;
//     constexpr sz M = K * 1024u;
//     constexpr sz G = M * 1024u;

//     const sz c = src.csts.size() * sizeof(irt::source::constant) +
//                  src.bins.size() * sizeof(irt::source::binary_file) +
//                  src.texts.size() * sizeof(irt::source::text_file);

//     if (c / G > 0)
//         ImGui::Text("Memory usage: %f Gb", ((double)c / (double)G));
//     else if (c / M > 0)
//         ImGui::Text("Memory usage: %f Mb", ((double)c / (double)M));
//     else
//         ImGui::Text("Memory usage: %f Kb", ((double)c / (double)K));
// }

void
application::show_sources(bool* is_show)
{
    ImGui::SetNextWindowPos(ImVec2(70, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

    static bool show_file_dialog = false;
    static irt::constant_source* constant_ptr = nullptr;
    static irt::binary_file_source* binary_file_ptr = nullptr;
    static irt::text_file_source* text_file_ptr = nullptr;

    if (!ImGui::Begin("External sources", is_show)) {
        ImGui::End();
        return;
    }

    // show_random();

    static ImGuiTableFlags flags =
      ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
      ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
      ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    if (ImGui::CollapsingHeader("List of constant sources")) {
        if (ImGui::BeginTable("Constant sources", 2, flags)) {
            ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("value",
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            small_string<16> label;
            static ImVector<u64> selection;

            constant_source* src = nullptr;
            while (srcs.constant_sources.next(src)) {
                const auto id = srcs.constant_sources.get_id(src);
                const auto index = get_index(id);
                const bool item_is_selected = selection.contains(ordinal(id));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                format(label, "{}", index);

                if (ImGui::Selectable(label.c_str(),
                                      item_is_selected,
                                      ImGuiSelectableFlags_AllowItemOverlap |
                                        ImGuiSelectableFlags_SpanAllColumns)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (item_is_selected)
                            selection.find_erase_unsorted(ordinal(id));
                        else
                            selection.push_back(ordinal(id));
                    } else {
                        selection.clear();
                        selection.push_back(ordinal(id));
                    }
                }
                ImGui::TableNextColumn();
                ImGui::PushID(ordinal(id));
                ImGui::InputDouble("##cell", &elem.second.value);
                ImGui::PopID();
            }

            ImGui::EndTable();

            if (ImGui::Button("New constant source"))
                constant_ptr = srcs.new_constant_source(0);

            ImGui::SameLine();
            if (ImGui::Button("Delete##constant")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    csts.erase(selection[i]);

                selection.clear();
            }
        }
    }

    if (ImGui::CollapsingHeader("List of binary file sources")) {
        if (ImGui::BeginTable("Binary files sources", 2, flags)) {
            ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            small_string<16> label;
            static ImVector<int> selection;

            binary_file_source* src = nullptr;
            while (srcs.binary_file_sources.next(src)) {
                const auto id = srcs.binary_file_sources.get_id(src);
                const auto index = get_index(id);
                const bool item_is_selected = selection.contains(ordinal(id));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                format(label, "{}", index);

                if (ImGui::Selectable(label.c_str(),
                                      item_is_selected,
                                      ImGuiSelectableFlags_AllowItemOverlap |
                                        ImGuiSelectableFlags_SpanAllColumns)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (item_is_selected)
                            selection.find_erase_unsorted(elem.first);
                        else
                            selection.push_back(elem.first);
                    } else {
                        selection.clear();
                        selection.push_back(elem.first);
                    }
                }
                ImGui::TableNextColumn();
                ImGui::PushID(elem.first);
                ImGui::Text(elem.second.file_path.string().c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (ImGui::Button("New binary source")) {
                binary_file_ptr = new_binary_file();
                show_file_dialog = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##binary")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    bins.erase(selection[i]);

                selection.clear();
            }
        }
    }

    if (ImGui::CollapsingHeader("List of text file sources")) {
        if (ImGui::BeginTable("Text files sources", 2, flags)) {
            ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            small_string<16> label;
            static ImVector<int> selection;

            for (auto& elem : texts) {
                const bool item_is_selected = selection.contains(elem.first);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                fmt::format_to_n(
                  label.begin(), label.capacity(), "{}", elem.first);
                if (ImGui::Selectable(label.c_str(),
                                      item_is_selected,
                                      ImGuiSelectableFlags_AllowItemOverlap |
                                        ImGuiSelectableFlags_SpanAllColumns)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (item_is_selected)
                            selection.find_erase_unsorted(elem.first);
                        else
                            selection.push_back(elem.first);
                    } else {
                        selection.clear();
                        selection.push_back(elem.first);
                    }
                }
                ImGui::TableNextColumn();
                ImGui::Text(elem.second.file_path.string().c_str());
                ImGui::PushID(elem.first);
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (ImGui::Button("New text source")) {
                text_file_ptr = new_text_file();
                show_file_dialog = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##text")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    texts.erase(selection[i]);

                selection.clear();
            }
        }
    }

    if (show_file_dialog) {
        if (binary_file_ptr) {
            const char* title = "Select file path to load";
            const char8_t* filters[] = { u8".dat", nullptr };

            ImGui::OpenPopup(title);
            if (load_file_dialog(binary_file_ptr->file_path, title, filters)) {
                show_file_dialog = false;
                binary_file_ptr = nullptr;
            }
        }

        if (text_file_ptr) {
            const char* title = "Select file path to load";
            const char8_t* filters[] = { u8".txt", nullptr };

            ImGui::OpenPopup(title);
            if (load_file_dialog(text_file_ptr->file_path, title, filters)) {
                show_file_dialog = false;
                text_file_ptr = nullptr;
            }
        }
    }

    size_in_bytes(*this);

    ImGui::End();
}

void
application::show_menu(const char* title, source& src)
{
    small_string<16> tmp;

    std::map<u64, constant_source>::value_type* constant_ptr = nullptr;
    std::map<u64, binary_file_source>::value_type* binary_file_ptr = nullptr;
    std::map<u64, text_file_source>::value_type* text_file_ptr = nullptr;
    std::map<u64, random_source>::value_type* random_ptr = nullptr;

    if (ImGui::BeginPopup(title)) {
        if (!srcs.constant_sources.empty() && ImGui::BeginMenu("Constant")) {
            for (auto& elem : srcs.constant_sources) {
                fmt::format_to_n(tmp.begin(), tmp.capacity(), "{}", elem.first);
                if (ImGui::MenuItem(tmp.c_str())) {
                    constant_ptr = &elem;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (!srcs.binary_file_sources.empty() &&
            ImGui::BeginMenu("Binary files")) {
            for (auto& elem : srcs.binary_file_sources) {
                fmt::format_to_n(tmp.begin(), tmp.capacity(), "{}", elem.first);
                if (ImGui::MenuItem(tmp.c_str())) {
                    binary_file_ptr = &elem;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (!srcs.text_file_sources.empty() && ImGui::BeginMenu("Text files")) {
            for (auto& elem : srcs.text_file_sources) {
                fmt::format_to_n(tmp.begin(), tmp.capacity(), "{}", elem.first);
                if (ImGui::MenuItem(tmp.c_str())) {
                    text_file_ptr = &elem;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (!srcs.random_sources.empty() && ImGui::BeginMenu("Random")) {
            for (auto& elem : srcs.random_sources) {
                fmt::format_to_n(tmp.begin(), tmp.capacity(), "{}", elem.first);
                if (ImGui::MenuItem(tmp.c_str())) {
                    random_ptr = &elem;
                    break;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    if (constant_ptr) {
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.client = 0;
        src.operation = constant_ptr;
    }

    if (binary_file_ptr) {
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.client = 0;
        src.operation = binary_file_ptr;
    }

    if (text_file_ptr) {
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.client = 0;
        src.operation = text_file_ptr;
    }

    if (random_ptr) {
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.client = 0;
        src.operation = random_ptr;
    }
}

} // namespace irt
