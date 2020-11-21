#if !defined(_SCOREP_PLUGIN_MPI_H_)
#define _SCOREP_PLUGIN_MPI_H_

#include <scorep/plugin/plugin.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <mpi_t_sampling.h>
#include <plugin_types.h>

using namespace scorep::plugin::policy;
using ThreadId = std::thread::id;
using TimeValuePair = std::pair<scorep::chrono::ticks, double>;
using MetricProperty = scorep::plugin::metric_property;
using ThreadEventPair = std::tuple<ThreadId, std::string>;


class scorep_plugin_mpi : public scorep::plugin::base<scorep_plugin_mpi,
    sync_strict, per_thread, scorep_clock>
{
    public:
        scorep_plugin_mpi();

        ~scorep_plugin_mpi();

        std::vector<MetricProperty>
        get_metric_properties(const std::string& metric_name);

        int32_t
        add_metric(const std::string& metric);

        void
        start();

        void
        stop();

        template <typename Proxy>
        void get_current_value(int32_t id, Proxy& proxy);

        template <typename Proxy>
        void get_optional_value(int32_t id, Proxy& proxy);

    private:
        std::mutex buffer_mutex_;
        std::size_t buffer_size_ = 0;
        mpi_t_sampling mpi_t_sampling_object;

        int m_mpi_t_initialized;

        size_t m_num_pvars;
};


template <typename Proxy>
void
scorep_plugin_mpi::get_current_value(int32_t id, Proxy& proxy)
{
    /* Enumerate PVARs */
    if (__builtin_expect(m_mpi_t_initialized, 1)) {
        /* Record current MPI_T value */
        proxy.write( (uint64_t)mpi_t_sampling_object.MPI_T_pvar_current_value_get(id) );
        return;
    }
    else {
      int ret;
      int flag;

      ret = mpi_t_sampling_object.MPI_Initialized(&flag);
      if (flag) {
        //std::cout << "Calling pvars enum()\n";
        mpi_t_sampling_object.MPI_T_pvars_enumerate();

        m_mpi_t_initialized = 1;
      }
    }

    uint64_t val = 0;
    proxy.write(val);
}


template <typename Proxy>
void
scorep_plugin_mpi::get_optional_value(int32_t id, Proxy& proxy)
{
    get_current_value(id, proxy);
}

#endif /* _SCOREP_PLUGIN_MPI_H_ */
