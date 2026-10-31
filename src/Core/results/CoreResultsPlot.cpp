#include "CoreResultsPlot.hpp"
#include "PlotChart.hpp"

CoreResultsPlot::CoreResultsPlot(CoreResults& results)
  : frd_query(results), dat_query(results)
{
}

bool CoreResultsPlot::show_plot(
  QString windowtitle, QString title, QString x_axis, QString y_axis,
  const std::vector<double>& x_data, const std::vector<double>& y_data,
  bool save, QString save_filepath)
{
  if (x_data.empty() || y_data.empty() || x_data.size() != y_data.size())
  {
    return false;
  }

  auto* chart = new PlotChart(nullptr, windowtitle, title, x_axis, y_axis, x_data, y_data, save, save_filepath);
  chart->show();

  if (save)
  {
    chart->close();
  }

  return true;
}

bool CoreResultsPlot::result_plot_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  std::vector<int> increments;
  std::vector<double> times;
  QString windowtitle = "FRD Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  increments = frd_query.frd_get_total_increments(job_id);
  for (size_t i = 0; i < increments.size(); i++)
  {
    times.push_back(frd_query.frd_get_time_from_total_increment(job_id,increments[i]));
  }

  //x data
  if (x_increment)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      x_data.push_back(increments[i]);
    }
  }
  if (x_time)
  {
    for (size_t i = 0; i < times.size(); i++)
    {
      x_data.push_back(times[i]);
    }
  }
  if (x_node_id!=-1)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      x_data.push_back(frd_query.frd_get_node_value(job_id,x_node_id, increments[i], x_block_type,x_block_component));
    }
  }

  //y data
  if (y_increment)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      y_data.push_back(increments[i]);
    }
  }
  if (y_time)
  {
    for (size_t i = 0; i < times.size(); i++)
    {
      y_data.push_back(times[i]);
    }
  }
  if (y_node_id!=-1)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      y_data.push_back(frd_query.frd_get_node_value(job_id,y_node_id, increments[i], y_block_type,y_block_component));
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_increment)
    {
      tmp = "Increment";
    }
    if (x_time)
    {
      tmp = "Time";
    }
    if (x_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(x_node_id) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_increment)
    {
      tmp = "Increment";
    }
    if (y_time)
    {
      tmp = "Time";
    }
    if (y_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(y_node_id) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }


  return show_plot(windowtitle, title, x_axis, y_axis, x_data, y_data, save, save_filepath);
}

bool CoreResultsPlot::result_plot_job_dat(int job_id,int x_node_id,int x_element_id,int x_element_ip, std::string x_block_set,std::string x_block_type, std::string x_block_component, bool x_time,int y_node_id,int y_element_id,int y_element_ip, std::string y_block_set,std::string y_block_type, std::string y_block_component, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  std::vector<double> x_times;
  std::vector<double> y_times;
  QString windowtitle = "Dat Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if (x_time&&y_time) //can't show time over time
  {
    return false;
  }
  //get times
  if (x_time)
  {
    x_times = dat_query.dat_get_result_block_times(job_id,y_block_type,y_block_set);
    y_times = dat_query.dat_get_result_block_times(job_id,y_block_type,y_block_set);
    x_data = x_times;
  }
  if (y_time)
  {
    x_times = dat_query.dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_times = dat_query.dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_data = y_times;
  }
  if ((!x_time)&&(!y_time))
  {
    x_times = dat_query.dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_times = dat_query.dat_get_result_block_times(job_id,y_block_type,y_block_set);
  }

  //x data
  if (x_node_id!=-1)
  {
    for (size_t i = 0; i < x_times.size(); i++)
    {
      x_data.push_back(dat_query.dat_get_node_value(job_id, x_node_id,x_times[i],x_block_type,x_block_set, x_block_component));
    }
  }
  if (x_element_id!=-1)
  {
    for (size_t i = 0; i < x_times.size(); i++)
    {
      std::vector<double> element_result = dat_query.dat_get_element_values_for_component(job_id, x_element_id,x_times[i],x_block_type,x_block_set, x_block_component);
      for (size_t ii = 0; ii < element_result.size(); ii++)
      {
        if (ii+1==x_element_ip)
        {
          x_data.push_back(element_result[ii]);
        }
      }
    }
  }

  //y data
  if (y_node_id!=-1)
  {
    for (size_t i = 0; i < y_times.size(); i++)
    {
      y_data.push_back(dat_query.dat_get_node_value(job_id, y_node_id,y_times[i],y_block_type,y_block_set, y_block_component));
    }
  }
  if (y_element_id!=-1)
  {
    for (size_t i = 0; i < y_times.size(); i++)
    {
      std::vector<double> element_result = dat_query.dat_get_element_values_for_component(job_id, y_element_id,y_times[i],y_block_type,y_block_set, y_block_component);
      for (size_t ii = 0; ii < element_result.size(); ii++)
      {
        if (ii+1==y_element_ip)
        {
          y_data.push_back(element_result[ii]);
        }
      }
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_time)
    {
      tmp = "Time";
    }
    if (x_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(x_node_id) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    if (x_element_id!=-1)
    {
      tmp = "Element ID " + std::to_string(x_element_id) + ", Int. Point "+ std::to_string(x_element_ip) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_time)
    {
      tmp = "Time";
    }
    if (y_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(y_node_id) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    if (y_element_id!=-1)
    {
      tmp = "Element ID " + std::to_string(y_element_id) + ", Int. Point "+ std::to_string(y_element_ip) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }

  /*
  std::string log;
  log = "x_times.size() "+ std::to_string(x_times.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "y_times.size() "+ std::to_string(y_times.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "x_data.size() "+ std::to_string(x_data.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "y_data.size() "+ std::to_string(y_data.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  */

  return show_plot(windowtitle, title, x_axis, y_axis, x_data, y_data, save, save_filepath);
}

bool CoreResultsPlot::result_plot_job_dat_section(int job_id, std::string x_section_set, std::string x_section_component, bool x_time, std::string y_section_set, std::string y_section_component, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  std::vector<double> x_times;
  std::vector<double> y_times;
  QString windowtitle = "Dat Section Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if (x_time&&y_time) //can't show time over time
  {
    return false;
  }

  std::vector<std::vector<double>> section_data;
  std::vector<std::string> components;
  components = dat_query.dat_get_section_label(job_id);
  int component_id = -1;

  if (x_time)
  {
    // get results
    section_data = dat_query.dat_get_section_data(job_id,y_section_set);

    //prepare times
    for (size_t i = 0; i < section_data.size(); i++)
    {
      x_times.push_back(section_data[i][0]);
    }

    x_data = x_times;

    for (size_t i = 0; i < components.size(); i++)
    {
      if (components[i]==y_section_component)
      {
        component_id = int(i);
        break;
      }
    }
  }

  if (y_time)
  {
    // get results
    section_data = dat_query.dat_get_section_data(job_id,x_section_set);

    //prepare times
    for (size_t i = 0; i < section_data.size(); i++)
    {
      y_times.push_back(section_data[i][0]);
    }

    y_data = y_times;

    for (size_t i = 0; i < components.size(); i++)
    {
      if (components[i]==x_section_component)
      {
        component_id = int(i);
        break;
      }
    }
  }

  if (component_id==-1) //requested component not found
  {
    return false;
  }

  //x data
  if (!x_time)
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      x_data.push_back(section_data[i][component_id]);
    }
  }

  //y data
  if (!y_time)
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      y_data.push_back(section_data[i][component_id]);
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_time)
    {
      tmp = "Time";
    }
    else
    {
      tmp = "Section Set " + x_section_set + " [" + x_section_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_time)
    {
      tmp = "Time";
    }
    else
    {
      tmp = "Section Set " + y_section_set + " [" + y_section_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }

  return show_plot(windowtitle, title, x_axis, y_axis, x_data, y_data, save, save_filepath);
}

