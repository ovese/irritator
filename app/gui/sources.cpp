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

static void
show_random_distribution_text(const random_source& src) noexcept
{
    switch (src.distribution) {
    case distribution_type::uniform_int:
        ImGui::Text("a: %d", src.a32);
        ImGui::Text("b: %d", src.b32);
        break;

    case distribution_type::uniform_real:
        ImGui::Text("a: %f", src.a);
        ImGui::Text("b: %f", src.b);
        break;

    case distribution_type::bernouilli:
        ImGui::Text("p: %f", src.p);
        break;

    case distribution_type::binomial:
        ImGui::Text("p: %f", src.p);
        ImGui::Text("t: %d", src.t32);
        break;

    case distribution_type::negative_binomial:
        ImGui::Text("p: %f", src.p);
        ImGui::Text("t: %d", src.k32);
        break;

    case distribution_type::geometric:
        ImGui::Text("p: %f", src.p);
        break;

    case distribution_type::poisson:
        ImGui::Text("mean: %f", src.mean);
        break;

    case distribution_type::exponential:
        ImGui::Text("lambda: %f", src.lambda);
        break;

    case distribution_type::gamma:
        ImGui::Text("alpha: %f", src.alpha);
        ImGui::Text("beta: %f", src.beta);
        break;

    case distribution_type::weibull:
        ImGui::Text("a: %f", src.a);
        ImGui::Text("b: %f", src.b);
        break;

    case distribution_type::exterme_value:
        ImGui::Text("a: %f", src.a);
        ImGui::Text("b: %f", src.b);
        break;

    case distribution_type::normal:
        ImGui::Text("mean: %f", src.mean);
        ImGui::Text("stddev: %f", src.stddev);
        break;

    case distribution_type::lognormal:
        ImGui::Text("m: %f", src.m);
        ImGui::Text("s: %f", src.s);
        break;

    case distribution_type::chi_squared:
        ImGui::Text("n: %f", src.n);
        break;

    case distribution_type::cauchy:
        ImGui::Text("a: %f", src.a);
        ImGui::Text("b: %f", src.b);
        break;

    case distribution_type::fisher_f:
        ImGui::Text("m: %f", src.m);
        ImGui::Text("s: %f", src.n);
        break;

    case distribution_type::student_t:
        ImGui::Text("n: %f", src.n);
        break;
    }
}

static void
show_random_distribution_input(random_source& src) noexcept
{
    static int current_item = 0;
    static u64 size = 1024u * 1024u;

    if (ImGui::CollapsingHeader("Random source")) {
        ImGui::InputScalar("length", ImGuiDataType_U64, &size);

        int old_current = current_item;
        ImGui::Combo("Distribution",
                     &current_item,
                     irt::distribution_type_str,
                     IM_ARRAYSIZE(irt::distribution_type_str));

        src.distribution = enum_cast<distribution_type>(current_item);

        switch (src.distribution) {
        case distribution_type::uniform_int: {
            if (old_current != current_item) {
                src.a32 = 0;
                src.b32 = 100;
            }

            int a = src.a32;
            int b = src.b32;

            if (ImGui::InputInt("a", &a)) {
                if (a < b)
                    src.a32 = a;
            }

            if (ImGui::InputInt("b", &b)) {
                if (a < b)
                    src.b32 = b;
            }
        } break;

        case distribution_type::uniform_real:
            if (old_current != current_item) {
                src.a = 0.0;
                src.b = 1.0;
            }
            ImGui::InputDouble("a", &src.a);
            ImGui::InputDouble("b", &src.b); // a < b
            break;

        case distribution_type::bernouilli:
            if (old_current != current_item) {
                src.p = 0.5;
            }
            ImGui::InputDouble("p", &src.p);
            break;

        case distribution_type::binomial:
            if (old_current != current_item) {
                src.p = 0.5;
                src.t32 = 1;
            }
            ImGui::InputDouble("p", &src.p);
            ImGui::InputInt("t", &src.t32);
            break;

        case distribution_type::negative_binomial:
            if (old_current != current_item) {
                src.p = 0.5;
                src.t32 = 1;
            }
            ImGui::InputDouble("p", &src.p);
            ImGui::InputInt("t", &src.k32);
            break;

        case distribution_type::geometric:
            if (old_current != current_item) {
                src.p = 0.5;
            }
            ImGui::InputDouble("p", &src.p);
            break;

        case distribution_type::poisson:
            if (old_current != current_item) {
                src.mean = 0.5;
            }
            ImGui::InputDouble("mean", &src.mean);
            break;

        case distribution_type::exponential:
            if (old_current != current_item) {
                src.lambda = 1.0;
            }
            ImGui::InputDouble("lambda", &src.lambda);
            break;

        case distribution_type::gamma:
            if (old_current != current_item) {
                src.alpha = 1.0;
                src.beta = 1.0;
            }
            ImGui::InputDouble("alpha", &src.alpha);
            ImGui::InputDouble("beta", &src.beta);
            break;

        case distribution_type::weibull:
            if (old_current != current_item) {
                src.a = 1.0;
                src.b = 1.0;
            }
            ImGui::InputDouble("a", &src.a);
            ImGui::InputDouble("b", &src.b);
            break;

        case distribution_type::exterme_value:
            if (old_current != current_item) {
                src.a = 1.0;
                src.b = 0.0;
            }
            ImGui::InputDouble("a", &src.a);
            ImGui::InputDouble("b", &src.b);
            break;

        case distribution_type::normal:
            if (old_current != current_item) {
                src.mean = 0.0;
                src.stddev = 1.0;
            }
            ImGui::InputDouble("mean", &src.mean);
            ImGui::InputDouble("stddev", &src.stddev);
            break;

        case distribution_type::lognormal:
            if (old_current != current_item) {
                src.m = 0.0;
                src.s = 1.0;
            }
            ImGui::InputDouble("m", &src.m);
            ImGui::InputDouble("s", &src.s);
            break;

        case distribution_type::chi_squared:
            if (old_current != current_item) {
                src.n = 1.0;
            }
            ImGui::InputDouble("n", &src.n);
            break;

        case distribution_type::cauchy:
            if (old_current != current_item) {
                src.a = 1.0;
                src.b = 0.0;
            }
            ImGui::InputDouble("a", &src.a);
            ImGui::InputDouble("b", &src.b);
            break;

        case distribution_type::fisher_f:
            if (old_current != current_item) {
                src.m = 1.0;
                src.n = 1.0;
            }
            ImGui::InputDouble("m", &src.m);
            ImGui::InputDouble("s", &src.n);
            break;

        case distribution_type::student_t:
            if (old_current != current_item) {
                src.n = 1.0;
            }
            ImGui::InputDouble("n", &src.n);
            break;
        }
    }
}

// static void
// size_in_bytes(const sources& src) noexcept
//{
//    constexpr sz K = 1024u;
//    constexpr sz M = K * 1024u;
//    constexpr sz G = M * 1024u;
//
//    const sz c = src.csts.size() * sizeof(irt::source::constant) +
//                 src.bins.size() * sizeof(irt::source::binary_file) +
//                 src.texts.size() * sizeof(irt::source::text_file);
//
//    if (c / G > 0)
//        ImGui::Text("Memory usage: %f Gb", ((double)c / (double)G));
//    else if (c / M > 0)
//        ImGui::Text("Memory usage: %f Mb", ((double)c / (double)M));
//    else
//        ImGui::Text("Memory usage: %f Kb", ((double)c / (double)K));
//}

void
application::show_sources(bool* is_show)
{
    ImGui::SetNextWindowPos(ImVec2(70, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

    static bool show_file_dialog = false;
    static irt::constant_source* constant_ptr = nullptr;
    static irt::binary_file_source* binary_file_ptr = nullptr;
    static irt::text_file_source* text_file_ptr = nullptr;
    static irt::random_source* random_source_ptr = nullptr;

    if (!ImGui::Begin("External sources", is_show)) {
        ImGui::End();
        return;
    }

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
                ImGui::PushID(get_index(id));
                ImGui::InputDouble("##cell", &src->buffer[0]);
                ImGui::PopID();
            }

            ImGui::EndTable();

            if (ImGui::Button("New constant source")) {
                if (srcs.constant_sources.can_alloc(1u)) {
                    auto& new_src = srcs.constant_sources.alloc();
                    constant_ptr = &new_src;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##constant")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    srcs.constant_sources.free(
                      enum_cast<constant_source_id>(selection[i]));

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
            static ImVector<u64> selection;

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
                            selection.find_erase_unsorted(ordinal(id));
                        else
                            selection.push_back(ordinal(id));
                    } else {
                        selection.clear();
                        selection.push_back(ordinal(id));
                    }
                }
                ImGui::TableNextColumn();
                ImGui::PushID(index);
                ImGui::Text(src->file_path.string().c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (ImGui::Button("New binary source")) {
                if (srcs.binary_file_sources.can_alloc(1u)) {
                    auto& new_src = srcs.binary_file_sources.alloc();
                    binary_file_ptr = &new_src;
                    show_file_dialog = true;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##binary")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    srcs.binary_file_sources.free(
                      enum_cast<binary_file_source_id>(selection[i]));

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
            static ImVector<u64> selection;

            text_file_source* src = nullptr;
            while (srcs.text_file_sources.next(src)) {
                const auto id = srcs.text_file_sources.get_id(src);
                const auto index = get_index(id);
                const bool item_is_selected = selection.contains(ordinal(id));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                format(label, "{}", ordinal(id));
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
                ImGui::Text(src->file_path.string().c_str());
                ImGui::PushID(index);
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (ImGui::Button("New text source")) {
                if (srcs.text_file_sources.can_alloc(1u)) {
                    auto& new_src = srcs.text_file_sources.alloc();
                    text_file_ptr = &new_src;
                    show_file_dialog = true;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##text")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    srcs.text_file_sources.free(
                      enum_cast<text_file_source_id>(selection[i]));

                selection.clear();
            }
        }
    }

    if (ImGui::CollapsingHeader("List of random sources")) {
        if (ImGui::BeginTable("Random sources", 2, flags)) {
            ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            small_string<16> label;
            static ImVector<u64> selection;

            random_source* src = nullptr;
            while (srcs.random_sources.next(src)) {
                const auto id = srcs.random_sources.get_id(src);
                const auto index = get_index(id);
                const bool item_is_selected = selection.contains(ordinal(id));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                format(label, "{}", ordinal(id));
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

                ImGui::Text("distribution: %s",
                            distribution_type_str[ordinal(src->distribution)]);
                show_random_distribution_input(*src);
            }
            ImGui::EndTable();

            if (ImGui::Button("New random source")) {
                if (srcs.random_sources.can_alloc(1u)) {
                    auto& new_src = srcs.random_sources.alloc();
                    random_source_ptr = &new_src;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete##text")) {
                for (int i = 0, e = selection.size(); i != e; ++i)
                    srcs.random_sources.free(
                      enum_cast<random_source_id>(selection[i]));

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

    // size_in_bytes(*this);

    ImGui::End();
}

void
application::show_menu_sources(const char* title,
                               external_source& srcs,
                               source& src)
{
    small_string<16> tmp;

    constant_source* constant_ptr = nullptr;
    binary_file_source* binary_file_ptr = nullptr;
    text_file_source* text_file_ptr = nullptr;
    random_source* random_ptr = nullptr;

    if (ImGui::BeginPopup(title)) {
        if (ImGui::BeginMenu("Constant")) {
            constant_source* s = nullptr;
            while (srcs.constant_sources.next(s)) {
                const auto id = srcs.constant_sources.get_id(s);
                const auto index = get_index(id);

                format(tmp, "{}", index);
                if (ImGui::MenuItem(tmp.c_str())) {
                    constant_ptr = s;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Binary files")) {
            binary_file_source* s = nullptr;
            while (srcs.binary_file_sources.next(s)) {
                const auto id = srcs.binary_file_sources.get_id(s);
                const auto index = get_index(id);

                format(tmp, "{}", index);
                if (ImGui::MenuItem(tmp.c_str())) {
                    binary_file_ptr = s;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Text files")) {
            text_file_source* s = nullptr;
            while (srcs.text_file_sources.next(s)) {
                const auto id = srcs.text_file_sources.get_id(s);
                const auto index = get_index(id);

                format(tmp, "{}", index);
                if (ImGui::MenuItem(tmp.c_str())) {
                    text_file_ptr = s;
                    break;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Random")) {
            random_source* s = nullptr;
            while (srcs.random_sources.next(s)) {
                const auto id = srcs.random_sources.get_id(s);
                const auto index = get_index(id);

                format(tmp, "{}", index);
                if (ImGui::MenuItem(tmp.c_str())) {
                    random_ptr = s;
                    break;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    if (constant_ptr) {
        src.reset();
        (*constant_ptr)(src, source::operation_type::initialize);
    }

    if (binary_file_ptr) {
        src.reset();
        (*binary_file_ptr)(src, source::operation_type::initialize);
    }

    if (text_file_ptr) {
        src.reset();
        (*text_file_ptr)(src, source::operation_type::initialize);
    }

    if (random_ptr) {
        src.reset();
        (*random_ptr)(src, source::operation_type::initialize);
    }
}

} // namespace irt
