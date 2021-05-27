// Copyright (c) 2020-2021 INRA Distributed under the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ORG_VLEPROJECT_IRRITATOR_EXTERNAL_SOURCE_2021
#define ORG_VLEPROJECT_IRRITATOR_EXTERNAL_SOURCE_2021

#include <irritator/core.hpp>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <random>
#include <vector>

namespace irt {

enum class external_source_type
{
    binary_file,
    constant,
    random,
    text_file
};

static inline const char* external_source_str[] = { "binary_file",
                                                    "constant",
                                                    "random",
                                                    "text_file" };

enum class distribution_type
{
    bernouilli,
    binomial,
    cauchy,
    chi_squared,
    exponential,
    exterme_value,
    fisher_f,
    gamma,
    geometric,
    lognormal,
    negative_binomial,
    normal,
    poisson,
    student_t,
    uniform_int,
    uniform_real,
    weibull,
};

static inline const char* distribution_type_str[] = {
    "bernouilli",        "binomial", "cauchy",  "chi_squared", "exponential",
    "exterme_value",     "fisher_f", "gamma",   "geometric",   "lognormal",
    "negative_binomial", "normal",   "poisson", "student_t",   "uniform_int",
    "uniform_real",      "weibull"
};

struct constant_source
{
    small_string<23> name;
    std::vector<double> buffer;

    status operator()(source& src, source::operation_type /*op*/) noexcept
    {
        if (buffer.empty())
            buffer.resize(1, 0.0);

        src.buffer = buffer.data();
        src.size = 1;
        src.index = 0;
        src.step = 0;
        src.type = ordinal(external_source_type::constant);
        src.id = 0;

        return status::success;
    }
};

struct binary_file_source
{
    small_string<23> name;
    std::vector<double> buffer;
    std::filesystem::path file_path;
    std::ifstream ifs;
    sz buffer_index = 0;

    status init(source& src)
    {
        if (!ifs) {
            ifs.open(file_path);

            if (!ifs)
                return status::success; /* to be fix */
        } else {
            ifs.seekg(0);
        }

        buffer.clear();
        buffer_index = 0;
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.type = ordinal(external_source_type::binary_file);
        src.id = 0;

        return status::success;
    }

    status finalize(source& src)
    {
        buffer.clear();
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;

        return status::success;
    }

    status operator()(source& src, source::operation_type op)
    {
        switch (op) {
        case source::operation_type::initialize:
            return init(src);

        case source::operation_type::update:
            return update(src);

        case source::operation_type::finalize:
            return finalize(src);
        }

        irt_unreachable();
    }

    status update(source& src)
    {
        if (!ifs.good())
            return status::success;

        return read(src);
    }

private:
    status read(source& src)
    {
        if (buffer_index + to_unsigned(src.size) > std::size(buffer)) {
            ifs.read(reinterpret_cast<char*>(buffer.data()), std::size(buffer));
            const auto read = ifs.gcount();
            buffer.resize(read);

            src.buffer = std::data(buffer);
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index = 512;
        } else {
            src.buffer = std::data(buffer) + buffer_index;
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index += 512;
        }

        return status::success;
    }
};

struct text_file_source
{
    small_string<23> name;
    std::vector<double> buffer;
    std::filesystem::path file_path;
    std::ifstream ifs;
    sz buffer_size = 0;
    sz buffer_index = 0;

    status init(source& src)
    {
        if (!ifs) {
            ifs.open(file_path);

            if (!ifs)
                return status::success; /* to be fix */
        } else {
            ifs.seekg(0);
        }

        buffer_size = 0;
        buffer_index = 0;
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.type = ordinal(external_source_type::text_file);
        src.id = 0;

        return status::success;
    }

    status finalize(source& src)
    {
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;

        return status::success;
    }

    status operator()(source& src, source::operation_type op)
    {
        switch (op) {
        case source::operation_type::initialize:
            return init(src);

        case source::operation_type::update:
            return update(src);

        case source::operation_type::finalize:
            return finalize(src);
        }

        irt_unreachable();
    }

    status update(source& src)
    {
        if (!ifs.good())
            return status::success;

        return read(src);
    }

private:
    status read(source& src)
    {
        if (buffer_index + to_unsigned(src.size) > std::size(buffer)) {
            size_t i = 0;
            for (; i < std::size(buffer) && ifs.good(); ++i) {
                if (!(ifs >> buffer[i]))
                    break;
            }

            buffer_size = i;

            src.buffer = std::data(buffer);
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index = 512;
        } else {
            src.buffer = std::data(buffer) + buffer_index;
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index += 512;
        }

        return status::success;
    }
};

struct random_source
{
    small_string<23> name;
    std::vector<double> buffer;
    sz buffer_size = 0;
    sz buffer_index = 0;
    distribution_type distribution = distribution_type::uniform_int;
    double a, b, p, mean, lambda, alpha, beta, stddev, m, s, n;
    int a32, b32, t32, k32;

    template<typename RandomGenerator, typename Distribution>
    void generate(RandomGenerator& gen, Distribution dist) noexcept
    {
        for (auto& elem : buffer)
            elem = dist(gen);
    }

    template<typename RandomGenerator>
    void generate(RandomGenerator& gen) noexcept
    {
        switch (distribution) {
        case distribution_type::uniform_int:
            generate(gen, std::uniform_int_distribution(a32, b32));
            break;

        case distribution_type::uniform_real:
            generate(gen, std::uniform_real_distribution(a, b));
            break;

        case distribution_type::bernouilli:
            generate(gen, std::bernoulli_distribution(p));
            break;

        case distribution_type::binomial:
            generate(gen, std::binomial_distribution(t32, p));
            break;

        case distribution_type::negative_binomial:
            generate(gen, std::negative_binomial_distribution(t32, p));
            break;

        case distribution_type::geometric:
            generate(gen, std::geometric_distribution(p));
            break;

        case distribution_type::poisson:
            generate(gen, std::poisson_distribution(mean));
            break;

        case distribution_type::exponential:
            generate(gen, std::exponential_distribution(lambda));
            break;

        case distribution_type::gamma:
            generate(gen, std::gamma_distribution(alpha, beta));
            break;

        case distribution_type::weibull:
            generate(gen, std::weibull_distribution(a, b));
            break;

        case distribution_type::exterme_value:
            generate(gen, std::extreme_value_distribution(a, b));
            break;

        case distribution_type::normal:
            generate(gen, std::normal_distribution(mean, stddev));
            break;

        case distribution_type::lognormal:
            generate(gen, std::lognormal_distribution(m, s));
            break;

        case distribution_type::chi_squared:
            generate(gen, std::chi_squared_distribution(n));
            break;

        case distribution_type::cauchy:
            generate(gen, std::cauchy_distribution(a, b));
            break;

        case distribution_type::fisher_f:
            generate(gen, std::fisher_f_distribution(m, n));
            break;

        case distribution_type::student_t:
            generate(gen, std::student_t_distribution(n));
            break;
        }
    }

    status init(source& src) noexcept
    {
        std::mt19937_64 gen;

        generate(gen);

        buffer_size = std::size(buffer);
        buffer_index = 0;
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
        src.type = ordinal(external_source_type::random);
        src.id = 0;

        return status::success;
    }

    status finalize(source& src)
    {
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;

        return status::success;
    }

    status operator()(source& src, source::operation_type op)
    {
        switch (op) {
        case source::operation_type::initialize:
            return init(src);

        case source::operation_type::update:
            return update(src);

        case source::operation_type::finalize:
            return finalize(src);
        }

        irt_unreachable();
    }

    status update(source& src)
    {
        const auto pos = buffer_index + to_unsigned(src.size);
        std::mt19937_64 gen;

        if (pos > buffer_size) {
            generate(gen);
            src.buffer = std::data(buffer);
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index = 512;
        } else {
            src.buffer = std::data(buffer) + pos;
            src.size = 512;
            src.index = 0;
            src.step = 1;
            buffer_index += 512;
        }

        return status::success;
    }
};

enum class constant_source_id : u64;
enum class binary_file_source_id : u64;
enum class text_file_source_id : u64;
enum class random_source_id : u64;

struct external_source
{
    data_array<constant_source, constant_source_id> constant_sources;
    data_array<binary_file_source, binary_file_source_id> binary_file_sources;
    data_array<text_file_source, text_file_source_id> text_file_sources;
    data_array<random_source, random_source_id> random_sources;
    std::mt19937_64 generator;

    status init(const sz size) noexcept
    {
        irt_return_if_bad(constant_sources.init(size));
        irt_return_if_bad(binary_file_sources.init(size));
        irt_return_if_bad(text_file_sources.init(size));
        irt_return_if_bad(random_sources.init(size));

        return status::success;
    }

    status operator()(source& src, const source::operation_type op) noexcept
    {
        if (src.type < 0 || src.type > 3)
            return status::success;

        const auto src_type = enum_cast<external_source_type>(src.type);
        switch (src_type) {
        case external_source_type::binary_file: {
            const auto src_id = enum_cast<binary_file_source_id>(src.id);
            if (auto* bin_src = binary_file_sources.try_to_get(src_id);
                bin_src) {
                return (*bin_src)(src, op);
            }
        } break;
        case external_source_type::constant: {
            const auto src_id = enum_cast<constant_source_id>(src.id);
            if (auto* cst_src = constant_sources.try_to_get(src_id); cst_src) {
                return (*cst_src)(src, op);
            }
        } break;

        case external_source_type::random: {
            const auto src_id = enum_cast<random_source_id>(src.id);
            if (auto* rnd_src = random_sources.try_to_get(src_id); rnd_src) {
                return (*rnd_src)(src, op);
            }
        } break;

        case external_source_type::text_file: {
            const auto src_id = enum_cast<text_file_source_id>(src.id);
            if (auto* txt_src = text_file_sources.try_to_get(src_id); txt_src) {
                return (*txt_src)(src, op);
            }
        } break;
        }

        return status::success;
    }
};

enum class random_file_type
{
    binary,
    text,
};

template<typename RandomGenerator, typename Distribution>
inline int
generate_random_file(std::ostream& os,
                     RandomGenerator& gen,
                     Distribution& dist,
                     const std::size_t size,
                     const random_file_type type) noexcept
{
    switch (type) {
    case random_file_type::text: {
        if (!os)
            return -1;

        for (std::size_t sz = 0; sz < size; ++sz)
            if (!(os << dist(gen) << '\n'))
                return -2;
    } break;

    case random_file_type::binary: {
        if (!os)
            return -1;

        for (std::size_t sz = 0; sz < size; ++sz) {
            const double value = dist(gen);
            os.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    } break;
    }

    return 0;
}

} // namespace irt

#endif
