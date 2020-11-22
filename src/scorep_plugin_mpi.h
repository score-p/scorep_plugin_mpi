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
    sync, per_thread, scorep_clock>
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

        /* Override, in order to set the delta_t */
        static SCOREP_Metric_Plugin_Info
        get_info()
        {
            SCOREP_Metric_Plugin_Info info =
                scorep::plugin::base<scorep_plugin_mpi, sync, per_thread, scorep_clock>::get_info();
            /* Update the delta_t */
            info.delta_t = 20000;
            return info;
        }

    private:
        std::mutex buffer_mutex_;
        std::size_t buffer_size_ = 0;
        mpi_t_sampling mpi_t_sampling_object;

        int m_mpi_t_initialized;

        size_t m_num_pvars;

        /* Decimate samples by this value */
        uint32_t m_mpi_samples_decimation;

        /* Decimation counter */
        uint32_t m_mpi_decimation_counter;

        /* Last read values */
        uint64_t *m_metrics_last_read_values;

        /* Set this value to 1 to indicate it's the read value duty cycle */
        uint32_t m_decimation_duty_cycle;
};


template <typename Proxy>
void
scorep_plugin_mpi::get_current_value(int32_t id, Proxy& proxy)
{
    /* Enumerate PVARs */
    if (__builtin_expect(m_mpi_t_initialized, 1)) {

        if (id == 0) {
            if ((m_mpi_decimation_counter & (m_mpi_samples_decimation-1)) == 0) {
                m_decimation_duty_cycle = 1;
            }
            else {
                m_decimation_duty_cycle = 0;
            }

            m_mpi_decimation_counter++;
        }

        //printf("id=%u, m_mpi_decimation_counter=%u, m_decimation_duty_cycle=%u, m_mpi_samples_decimation=%u\n",
        //    id, m_mpi_decimation_counter, m_decimation_duty_cycle, m_mpi_samples_decimation);

        /* Record current MPI_T value */
        if (m_decimation_duty_cycle) {
            m_metrics_last_read_values[id] =
                (uint64_t)mpi_t_sampling_object.MPI_T_pvar_current_value_get(id);
        }

        proxy.write( m_metrics_last_read_values[id] );

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
