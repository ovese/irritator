// Copyright (c) 2020 INRA Distributed under the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ORG_VLEPROJECT_IRRITATOR_source_2021
#define ORG_VLEPROJECT_IRRITATOR_source_2021

#include <irritator/core.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>

namespace irt::sources {

struct constant_source
{
    double buffer = 0;

    status operator()(source& src, source::operation_type /*op*/) noexcept
    {
        src.buffer = &buffer;
        src.size = 1;
        src.index = 0;
        src.step = 0;
        src.user_data = static_cast<void*>(this);

        return status::success;
    }
};

struct binary_file_source
{
    std::array<double, 1024 * 1024> buffer;
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

        return status::success;
    }

    status finalize(source& src)
    {
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
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
            buffer_size = ifs.gcount();

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
    std::array<double, 1024 * 1024 / 8> buffer;
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

        return status::success;
    }

    status finalize(source& src)
    {
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
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
    enum class distribution_type
    {
        uniform_int,
        uniform_real,
        bernouilli,
        binomial,
        negative_binomial,
        geometric,
        poisson,
        exponential,
        gamma,
        weibull,
        exterme_value,
        normal,
        lognormal,
        chi_squared,
        cauchy,
        fisher_f,
        student_t
    };

    std::array<double, 1024 * 1024> buffer;
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

        return status::success;
    }

    status finalize(source& src)
    {
        src.buffer = nullptr;
        src.size = 0;
        src.index = 0;
        src.step = 0;
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

enum class external_source_type
{
    external_source_binary_file = 0,
    external_source_constant,
    external_source_random,
    external_source_text_file
};

struct external_source
{
    std::map<u64, constant_source> constant_sources;
    std::map<u64, binary_file_source> binary_file_sources;
    std::map<u64, text_file_source> text_file_sources;
    std::map<u64, random_source> random_sources;

    status read(simulation& sim, source& src, std::istream& is)
    {
        static std::string_view names[] = {
            "binary-file", "constant", "random", "text-file"
        };

        char type[20];
        if (!(is >> type))
            return status::success; /* @todo fix errror */

        auto it = binary_find(
          names,
          names + std::size(names),
          type,
          [](const auto lhs, const auto rhs) { return lhs == rhs; });

        if (it == names + std::size(names))
            return status::success; /* @todo fix error */

        const auto type_id = std::distance(names, it);
        if (type_id == 0) {
            sz size;

            if (!(is >> size))
                return status::success; /* @todo fix errror */

            if (size == 0)
                return status::success; /* @todo fix errror */

            std::vector<double> data(size, 0.0);
            for (sz i = 0; i < size; ++i) {
                src.type = 0;

                if (!(is >> data[i]))
                    return status::success; /* @todo fix errror */
            }
        }
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

} // namespace irt::source

#endif
