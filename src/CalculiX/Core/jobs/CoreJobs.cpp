#include "Core/application/SettingsRead.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/application/CoreObserver.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/application/CoreIdSelection.hpp"
#include <string>
#include <vector>
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "Core/results/vtk/CoreResultsVtkStructs.hpp"
#include <QScopedValueRollback>
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "StopWatch.hpp"

#include "CubitProcess.hpp"
#include "CubitString.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include "CubitFile.hpp"

#include <stdlib.h>
#ifdef WIN32
 #include <windows.h>
 #include <io.h>
 #define BUFSIZE 4096
#else
 #include <unistd.h>
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <signal.h>
#endif
#include <fstream>
#include <chrono>
#include <iostream>
#include <sstream>
#include <limits>
#include <exception>
#include <cerrno>
#include "loadUserOptions.hpp"


CoreJobs::CoreJobs()
{
  #ifdef WIN32
    ProcessPipe.clear();
    PPTID.clear();
    PipePID.clear();
    PipeThreads.clear();
    PipeThreadsRun.clear();
  #endif
}

CoreJobs::~CoreJobs()
{
#ifdef WIN32
  for (const auto& retained : running_process_handles)
  {
    CloseHandle(retained.second);
  }
#endif
}

bool CoreJobs::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    progressbar = CubitInterface::app_util().get()->progress_tool();;
    is_initialized = true;  
    return true;
  }
}

bool CoreJobs::update()
{ 
  return true;
}

bool CoreJobs::reset()
{
  if (completing_results || CalculiXResultsInterface().is_vtk_build_active())
  {
    PRINT_ERROR("Cannot reset jobs during result preparation.\n");
    return false;
  }
  pending_result_jobs.clear();
  clear_job_progress();
  clear_monitor_executions();
  #ifdef WIN32
    //close all jobs and threads first
    
    for (size_t i = 0; i < jobs_data.size(); i++)
    {
      HANDLE processhandle;
      DWORD returnCode{};
      processhandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, std::stoi(jobs_data[i][4]));
      if (GetExitCodeProcess(processhandle, &returnCode)) {
        if (returnCode == STILL_ACTIVE)
        { 
          std::string log;
          log = "Kill Job " + jobs_data[i][1] + " with ID " + jobs_data[i][0] + " on Reset\n";
          PRINT_INFO("%s", log.c_str());
          TerminateProcess(processhandle, 1);
          CloseHandle(processhandle);
          kill_pipe(std::stoi(jobs_data[i][0]));
        }
      }
    }
    
    for (const auto& retained : running_process_handles)
    {
      CloseHandle(retained.second);
    }
    running_process_handles.clear();
    //clear all vectors
    ProcessPipe.clear();
    PPTID.clear();
    PipePID.clear();
    PipeThreads.clear();
    PipeThreadsRun.clear();
  #else
    CubitProcessHandler.clear();
  #endif
  jobs_data.clear();  
  output_console.clear();
  cvg.clear();
  sta.clear();

  init();
  return true;
}

bool CoreJobs::check_initialized()
{
  return is_initialized;
}

bool CoreJobs::create_job(std::vector<std::string> options)
{
  int created_job_id = -1;
  return create_job(options, created_job_id);
}

bool CoreJobs::create_job(const std::vector<std::string>& options, int& created_job_id)
{
  CalculiXResultsInterface ccx_results_iface;

  created_job_id = -1;

  if (!is_initialized || options.size() != 2 ||
      options[0].find_first_not_of(" \t\r\n") == std::string::npos)
  {
    PRINT_ERROR("Invalid job creation data.\n");
    return false;
  }

  int largest_id = 0;

  for (const auto& job : jobs_data)
  {
    if (job.empty())
    {
      PRINT_ERROR("Invalid existing job data.\n");
      return false;
    }

    int id = -1;
    std::istringstream input(job[0]);

    if (!(input >> id) || id <= 0)
    {
      PRINT_ERROR("Invalid existing job ID.\n");
      return false;
    }

    input >> std::ws;

    if (!input.eof())
    {
      PRINT_ERROR("Invalid existing job ID.\n");
      return false;
    }

    if (id > largest_id)
    {
      largest_id = id;
    }
  }

  if (largest_id == (std::numeric_limits<int>::max)())
  {
    PRINT_ERROR("No further job IDs are available.\n");
    return false;
  }

  const int job_id = largest_id + 1;

  if (!add_job(job_id, options[0], options[1]))
  {
    if (!ccx_results_iface.delete_result(job_id))
    {
      PRINT_ERROR("Failed to remove results for job %d.\n", job_id);
    }
    
    PRINT_ERROR("Failed to create job %d.\n", job_id);
    return false;
  }

  if (!ccx_results_iface.create_result(job_id))
  {
    PRINT_ERROR("Failed to create results for job %d.\n", job_id);
    return false;
  }

  created_job_id = job_id;
  return true;
}

bool CoreJobs::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  if (completing_results || CalculiXResultsInterface().is_vtk_build_active())
  {
    PRINT_ERROR("Cannot modify jobs during result preparation.\n");
    return false;
  }
  int jobs_data_id = get_jobs_data_id_from_job_id(job_id);
  
  if (jobs_data_id == -1)
  {
    return false;
  } else {
    // name
    if (options_marker[0]==1)
    {
      jobs_data[jobs_data_id][1] = options[0];
    }
    // filepath
    if (options_marker[1]==1)
    {
      jobs_data[jobs_data_id][2] = options[1];
    }
    return true;
  }
}

bool CoreJobs::add_job(int job_id, std::string name, std::string filepath)
{
  std::vector<std::string> v = {std::to_string(job_id), name, filepath, "-1", "-1","","-1","","","1"};      
  jobs_data.push_back(v);
  
  std::vector<std::string> tmp_output_console;
  output_console.push_back(tmp_output_console);
  std::vector<std::string> tmp_cvg;
  cvg.push_back(tmp_cvg);
  std::vector<std::string> tmp_sta;
  sta.push_back(tmp_sta);
  
  jobs_data[jobs_data.size()-1][5] = std::to_string(output_console.size()-1);
  jobs_data[jobs_data.size()-1][7] = std::to_string(cvg.size()-1);
  jobs_data[jobs_data.size()-1][8] = std::to_string(sta.size()-1);

  return true;
}

bool CoreJobs::delete_job(int job_id)
{
  if (completing_results || CalculiXResultsInterface().is_vtk_build_active())
  {
    PRINT_ERROR("Cannot delete jobs during result preparation.\n");
    return false;
  }
  CalculiXResultsInterface ccx_results_iface;

  int jobs_data_id = get_jobs_data_id_from_job_id(job_id);
  if (jobs_data_id == -1)
  {
    return false;
  } else {
    // todo erasing console output ect., should normally be not too much memory so....no prob for now
    CalculiXJobsInterface jobs_iface;
    int topology_id = -1;
    int iteration_id = -1;
    jobs_iface.get_job_topology_context(job_id, topology_id, iteration_id);
    jobs_data.erase(jobs_data.begin() + jobs_data_id);
    monitorExecutions.erase(job_id);
    job_progress.erase(job_id);
    pending_result_jobs.erase(job_id);
    ccx_results_iface.delete_result(job_id);
    auto& observer = CalculiXApplicationInterface().core_observer();
    if (observer.job_deleted)
    {
      observer.job_deleted(job_id);
    }
    if (topology_id > 0 && observer.topology_data_cleared)
    {
      observer.topology_data_cleared(topology_id);
    }
    return true;
  }
}

bool CoreJobs::run_job(int job_id,int option)
{
  // Observe completed processes before another run can overwrite their input
  // or output files. This also covers a restart before the GUI timer ticks.
  if (!check_jobs() || !results_ready())
  {
    PRINT_ERROR("Finish or retry pending VTK result conversion before starting another job.\n");
    return false;
  }
  CalculiXJobsInterface ccx_jobs_iface;
 
  #ifdef WIN32
    std::string filepath;
    std::string log;
    std::string command;
    CubitString programm;
    CubitString working_dir;
    CubitString temp;
    CubitString output;
    int process_id = 0;
    
    
    if (_access(ccx_uo.mPathSolver.toStdString().c_str(), 0) == 0)
    { 
      SetEnvironmentVariableA("OMP_NUM_THREADS",std::to_string(ccx_uo.mSolverThreads).c_str());
    }else{
      log = "CCX Solver not found! checked path \"" + ccx_uo.mPathSolver.toStdString() + "\" \n";
      PRINT_INFO("%s", log.c_str());    
      return false;
    }   
    
    int job_data_id;
    job_data_id = get_jobs_data_id_from_job_id(job_id);
    if (job_data_id != -1)
    { 
      // kill process if already running
      {
        HANDLE processhandle;
        DWORD returnCode{};
        processhandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, std::stoi(jobs_data[job_data_id][4]));
        if (GetExitCodeProcess(processhandle, &returnCode)) {
          if (returnCode == STILL_ACTIVE)
          { 
            log = "Kill Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " if already running \n";
            output_console[std::stoi(jobs_data[job_data_id][5])].clear();
            PRINT_INFO("%s", log.c_str());
            TerminateProcess(processhandle, 1);
            CloseHandle(processhandle);
            kill_pipe(job_id);
            log = " Job killed!\n";
            PRINT_INFO("%s", log.c_str());
            jobs_data[job_data_id][3] = "3";         
          }
        }
      }
      
      if (jobs_data[job_data_id][2]!="")
      {
        //std::string shellstr;
        //shellstr = "cp '" + jobs_data[job_data_id][2] + "' '" + jobs_data[job_data_id][1] + ".inp'";
        //system(shellstr.c_str());
        //PRINT_INFO("%s", shellstr.c_str());
        std::string SourcePath = jobs_data[job_data_id][2];
        filepath = jobs_data[job_data_id][1] + ".inp";
        CopyFileA( SourcePath.c_str(), filepath.c_str(), FALSE );
      } else {
        filepath = jobs_data[job_data_id][1] + ".inp";
        log = "Exporting Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " to \n";
        log.append(filepath +  " \n");
        PRINT_INFO("%s", log.c_str());
        if (!ccx_jobs_iface.prepare_job_input(job_id, filepath))
        {
          PRINT_ERROR("Failed to export input for job %d.\n", job_id);
          return false;
        }
      }

      HANDLE g_hChildStd_OUT_Rd = NULL;
      HANDLE g_hChildStd_OUT_Wr = NULL;
      SECURITY_ATTRIBUTES saAttr; 
      
      // Set the bInheritHandle flag so pipe handles are inherited. 
    
      saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
      saAttr.bInheritHandle = TRUE; 
      saAttr.lpSecurityDescriptor = NULL; 

      // Create a pipe for the child process's STDOUT. 
      CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);
      SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
      //DWORD mode= PIPE_READMODE_BYTE|PIPE_NOWAIT;
      //SetNamedPipeHandleState(g_hChildStd_OUT_Rd, &mode, NULL, NULL);
      
      PROCESS_INFORMATION piProcInfo; 
      STARTUPINFO siStartInfo;
      BOOL bSuccess = FALSE; 
    
      // Set up members of the PROCESS_INFORMATION structure. 
    
      ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    
      // Set up members of the STARTUPINFO structure. 
      // This structure specifies the STDIN and STDOUT handles for redirection.
    
      ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
      siStartInfo.cb = sizeof(STARTUPINFO); 
      siStartInfo.hStdError = g_hChildStd_OUT_Wr;
      siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
      siStartInfo.hStdInput = NULL;
      siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
      std::string arguments = "\"" + ccx_uo.mPathSolver.toStdString() + "\" -i \"" + filepath.substr(0, filepath.size()-4) + "\"";
      std::wstring args = std::wstring(arguments.begin(), arguments.end());
      //arguments = "\"C:\\Windows\\System32\\cmd.exe\" echo output && ping -n 10 127.0.0.1 >NUL";
      //PRINT_INFO("%s", arguments.c_str()); 

      //clear stored data
      output_console[std::stoi(jobs_data[job_data_id][5])].clear();
      job_progress.erase(job_id);
      cvg[std::stoi(jobs_data[job_data_id][7])].clear();
      sta[std::stoi(jobs_data[job_data_id][8])].clear();

      // Create the child process.         
      PRINT_INFO("Job %d: starting solver process.\n", job_id);
      bSuccess = CreateProcess(NULL, 
          //&arguments[0],     // command line 
          &args[0],     // command line 
          NULL,          // process security attributes 
          NULL,          // primary thread security attributes 
          TRUE,          // handles are inherited 
          CREATE_NO_WINDOW,   // creation flags 
          NULL,          // use parent's environment 
          NULL,          // use parent's current directory 
          &siStartInfo,  // STARTUPINFO pointer 
          &piProcInfo);  // receives PROCESS_INFORMATION

      // If an error occurs, exit the application. 
      if ( ! bSuccess )
      {
          log = "Running Job Failed! \n";
          PRINT_INFO("%s", log.c_str()); 
          return false;
      } else
      {        
        // Retain the process handle so even fast solvers remain queryable.
        running_process_handles.insert_or_assign(job_id, piProcInfo.hProcess);
        capture_monitor_execution(job_id, piProcInfo.dwProcessId);
        CloseHandle(piProcInfo.hThread);
        
        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        
        CloseHandle(g_hChildStd_OUT_Wr);

        jobs_data[job_data_id][4] = std::to_string(int(piProcInfo.dwProcessId));
        jobs_data[job_data_id][3] = "1";
        jobs_data[job_data_id][6] = "-1";
        jobs_data[job_data_id][9] = std::to_string(option);

        ProcessPipe.push_back(g_hChildStd_OUT_Rd);

        DWORD dwRead; 
        CHAR chBuf[BUFSIZE]; 
        ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        //std::string s(chBuf, dwRead);
        //log = "output: \n" + s + " \n";
        //PRINT_INFO("%s", log.c_str());
        std::string output(chBuf, dwRead);
        output_console[std::stoi(jobs_data[job_data_id][5])].push_back(output);
        
        /*
        log = "pipethreads \n" + std::to_string(PipeThreads.size()) + " \n";
        log.append("pipethreadsrun \n" + std::to_string(PipeThreadsRun.size()) + " \n");
        PRINT_INFO("%s", log.c_str());
        */

        PipeThreadsRun.push_back(true);
        int bool_data_id = int(PipeThreadsRun.size()-1);
        PipeThreads.push_back(std::thread(&CoreJobs::read_pipe, this, job_id, bool_data_id));

        /*
        log = "pipethreads \n" + std::to_string(PipeThreads.size()) + " \n";
        log.append("pipethreadsrun \n" + std::to_string(PipeThreadsRun.size()) + " \n");
        PRINT_INFO("%s", log.c_str());
        */

        // write linking of the ids
        PULONG ClientProcessId = 0;
        GetNamedPipeClientProcessId(g_hChildStd_OUT_Rd,ClientProcessId);
        std::ostringstream oss;
        oss << PipeThreads[PipeThreads.size()-1].get_id() << std::endl;
        std::vector<int> tmp;
        tmp.push_back(int(piProcInfo.dwProcessId));
        tmp.push_back(int(ClientProcessId));
        tmp.push_back(std::stoi(oss.str()));
        PPTID.push_back(tmp);
        PipePID.push_back(int(ClientProcessId));
        /*
        log = "CREATED THREAD ID " + oss.str() + " \n";
        log.append("NUMBER OF PIPE READ THREADS " + std::to_string(PipeThreads.size())  + " \n");
        PRINT_INFO("%s", log.c_str());
        */
      }

      return true;
    }

  #else
    std::string filepath;
    std::string log;
    std::string command;
    int CubitProcessHandler_data_id;
    CubitString programm;
    CubitString working_dir;
    CubitString temp;
    CubitString output;
    std::vector<CubitString> arguments(3);
    size_t process_id; 

    if (access(ccx_uo.mPathSolver.toStdString().c_str(), X_OK) == 0) 
    {
      setenv("OMP_NUM_THREADS",std::to_string(ccx_uo.mSolverThreads).c_str(),1);
    }else{
      log = "CCX Solver not found! checked path \"" + ccx_uo.mPathSolver.toStdString() + "\" \n";
      PRINT_INFO("%s", log.c_str());    
      return false;
    }

    int job_data_id;
    job_data_id = get_jobs_data_id_from_job_id(job_id);
    if (job_data_id != -1)
    {
      // create or get cubitprocess
      CubitProcessHandler_data_id = get_CubitProcessHandler_data_id_from_process_id(std::stoi(jobs_data[job_data_id][4]));
      if (CubitProcessHandler_data_id == -1)
      {
        CubitProcess newCubitProcess;
        CubitProcessHandler.push_back(newCubitProcess);
        CubitProcessHandler_data_id = int(CubitProcessHandler.size())-1;
      }else{
        log = "Kill Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " if already running \n";
        output_console[std::stoi(jobs_data[job_data_id][5])].clear();
        PRINT_INFO("%s", log.c_str());
        CubitProcessHandler[CubitProcessHandler_data_id].kill();
        CubitProcessHandler[CubitProcessHandler_data_id].wait();
        //log = " Job killed with Exit Code " + std::to_string(CubitProcessHandler[CubitProcessHandler_data_id].exit_code()) + " \n";
        log = " Job killed!\n";
        PRINT_INFO("%s", log.c_str());
        jobs_data[job_data_id][3] = "3";
      }

      if (jobs_data[job_data_id][2]!="")
      {
        std::string shellstr;
        shellstr = "cp '" + jobs_data[job_data_id][2] + "' '" +jobs_data[job_data_id][1] + ".inp'";
        system(shellstr.c_str());
        filepath = jobs_data[job_data_id][1] + ".inp";
      } else {
        filepath = jobs_data[job_data_id][1] + ".inp";
        log = "Exporting Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " to \n";
        log.append(filepath +  " \n");
        PRINT_INFO("%s", log.c_str());
        if (!ccx_jobs_iface.prepare_job_input(job_id, filepath))
        {
          PRINT_ERROR("Failed to export input for job %d.\n", job_id);
          return false;
        }
      }

      programm = ccx_uo.mPathSolver.toStdString().c_str();
      arguments[0] = "-i";
      arguments[1] = filepath.substr(0, filepath.size()-4);
      arguments[2] = NULL;
      
      CubitProcessHandler[CubitProcessHandler_data_id].set_program(programm);
      CubitProcessHandler[CubitProcessHandler_data_id].set_arguments(arguments);
      CubitProcessHandler[CubitProcessHandler_data_id].set_channel_mode(CubitProcess::ChannelMode::MergedChannels);
      temp = CubitProcessHandler[CubitProcessHandler_data_id].find_executable(programm);
      output_console[std::stoi(jobs_data[job_data_id][5])].clear();
      job_progress.erase(job_id);
      cvg[std::stoi(jobs_data[job_data_id][7])].clear();
      sta[std::stoi(jobs_data[job_data_id][8])].clear();

      PRINT_INFO("Job %d: starting solver process.\n", job_id);
      CubitProcessHandler[CubitProcessHandler_data_id].start();
      process_id = CubitProcessHandler[CubitProcessHandler_data_id].pid();
      if (process_id <= 0)
      {
        PRINT_ERROR("Failed to start solver for job %d.\n", job_id);
        return false;
      }
      if (process_id!=0)
      { 
        capture_monitor_execution(job_id, process_id);
        jobs_data[job_data_id][4] = std::to_string(process_id);
        jobs_data[job_data_id][3] = "1";
        jobs_data[job_data_id][6] = "-1";
        jobs_data[job_data_id][9] = std::to_string(option);
        /*
        log = " Path to executable ";
        log.append(working_dir.str() + temp.str() + "\n");
        log.append(temp.str() + "\n");
        log = " Output " + output.str() + " \n";
        PRINT_INFO("%s", log.c_str());
        */
      }
      return true;
    } else {
      return false;
    }  
  #endif 
  
  return false;
}

bool CoreJobs::wait_job(int job_id)
{
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXResultsInterface ccx_results_iface;
 
  bool results_loaded = false;
  
  #ifdef WIN32
    std::string log;
    CubitString output;
    
    int status=-1;
    
    int jobs_data_id=-1;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      int status = -1;
      HANDLE processhandle;
      DWORD returnCode{};
      processhandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, std::stoi(jobs_data[jobs_data_id][4]));
      if (GetExitCodeProcess(processhandle, &returnCode)) {
        if (returnCode == STILL_ACTIVE)
        { 
          log = " Waiting for Job " + jobs_data[jobs_data_id][1] + " to Exit \n";
          PRINT_INFO("%s", log.c_str());

          log = " Waiting for Job " + jobs_data[jobs_data_id][1] + " to Exit";
          progressbar->start(0,100,log.c_str());
          auto t_start = std::chrono::high_resolution_clock::now();          

          jobs_data[jobs_data_id][3] = "5"; // mark the job for waiting

    	    /*
          DWORD dwRead; 
          CHAR chBuf[BUFSIZE];
          bool success = true; 
        
          while (success) 
          {
            int ProcessPipe_data_id;
            int PipePID;
            PipePID = get_PipePID_from_ProcessPID(std::stoi(jobs_data[jobs_data_id][4]));           
            ProcessPipe_data_id = get_ProcessPipe_data_id_from_PipePID(PipePID);
            success = ReadFile(ProcessPipe[ProcessPipe_data_id], chBuf, BUFSIZE, &dwRead, NULL);
            std::string output(chBuf, dwRead);
            output_console[std::stoi(jobs_data[jobs_data_id][5])].push_back(output);
            get_cvgsta(std::stoi(jobs_data[jobs_data_id][0]));        
            //update progress bar
            const auto t_end = std::chrono::high_resolution_clock::now();
            int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
            //currentDataRow += frd->result_block_data[data_ids[ii]].size();
            if (duration > 500)
            {
              progressbar->check_interrupt();
              t_start = std::chrono::high_resolution_clock::now();
            }
          }
          progressbar->end();
          */

          while (returnCode == STILL_ACTIVE)
          {
            GetExitCodeProcess(processhandle, &returnCode);
            //update progress bar
            const auto t_end = std::chrono::high_resolution_clock::now();
            int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
            //currentDataRow += frd->result_block_data[data_ids[ii]].size();
            if (duration > 500)
            {
              progressbar->check_interrupt();
              t_start = std::chrono::high_resolution_clock::now();
            }
          }
          progressbar->end();
        }
      }

      if (GetExitCodeProcess(processhandle, &returnCode)) 
      {
        if (returnCode != STILL_ACTIVE)
        { 
          status = int(returnCode);
          // delete handles
          int ProcessPipe_data_id;
          int PipePID;
          PipePID = get_PipePID_from_ProcessPID(std::stoi(jobs_data[jobs_data_id][4]));           
          ProcessPipe_data_id = get_ProcessPipe_data_id_from_PipePID(PipePID);
          kill_pipe(job_id);
          if (ProcessPipe_data_id >= 0 &&
              static_cast<size_t>(ProcessPipe_data_id) < ProcessPipe.size())
          {
            CloseHandle(ProcessPipe[ProcessPipe_data_id]);
            ProcessPipe.erase(ProcessPipe.begin() + ProcessPipe_data_id);
            PPTID.erase(PPTID.begin() + ProcessPipe_data_id);
            this->PipePID.erase(this->PipePID.begin() + ProcessPipe_data_id);
          }
          const auto retained = running_process_handles.find(job_id);
          if (retained != running_process_handles.end())
          {
            CloseHandle(retained->second);
            running_process_handles.erase(retained);
          }
        }
      }
      if (status!=0)
      {
        log = "Job " + jobs_data[jobs_data_id][1] + " with ID " + jobs_data[jobs_data_id][0] + " exited with errors! \n";
        //log.append(" Exit Code " + std::to_string(errorcode) + " \n");
        PRINT_INFO("%s", log.c_str());
        jobs_data[jobs_data_id][3] = "4";
        jobs_data[jobs_data_id][6] = "-1";
      }else{
        log = "Job " + jobs_data[jobs_data_id][1] + " with ID " + jobs_data[jobs_data_id][0] + " finished! \n";
        //log.append(" Exit Code " + std::to_string(errorcode) + " \n");
        PRINT_INFO("%s", log.c_str());
        jobs_data[jobs_data_id][3] = "2";
        results_loaded = complete_job_results(job_id);
      }
    }
    const auto retained = running_process_handles.find(job_id);
    if (retained != running_process_handles.end())
    {
      CloseHandle(retained->second);
      running_process_handles.erase(retained);
    }
    return results_loaded && get_job_status(job_id) == 2;
  #else
    std::string log;
    CubitString output;
    int status=-1;
    
    int jobs_data_id=-1;
    int CubitProcessHandler_data_id;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      CubitProcessHandler_data_id = get_CubitProcessHandler_data_id_from_process_id(std::stoi(jobs_data[jobs_data_id][4]));
      if (CubitProcessHandler_data_id != -1)
      {
        log = " Waiting for Job " + jobs_data[jobs_data_id][1] + " to Exit \n";
        PRINT_INFO("%s", log.c_str());

        log = " Waiting for Job " + jobs_data[jobs_data_id][1] + " to Exit";
        progressbar->start(0,100,log.c_str());
        auto t_start = std::chrono::high_resolution_clock::now();          

        jobs_data[jobs_data_id][3] = "5"; // mark the job for waiting

        while (0 == kill(std::stoi(jobs_data[jobs_data_id][4]),0))
        {
          output = CubitProcessHandler[CubitProcessHandler_data_id].read_output_channel(-1);
          if (output != "")
          {
            log = " Output " + output.str() + " \n";
            //PRINT_INFO("%s", log.c_str());
            //jobs_data[jobs_data_id][5] = jobs_data[jobs_data_id][5] + output.str();
            output_console[std::stoi(jobs_data[jobs_data_id][5])].push_back(output.str());
            output = "";
            get_cvgsta(std::stoi(jobs_data[jobs_data_id][0]));
          }
          waitpid(std::stoi(jobs_data[jobs_data_id][4]), &status,WNOHANG);

          //update progress bar
          const auto t_end = std::chrono::high_resolution_clock::now();
          int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
          //currentDataRow += frd->result_block_data[data_ids[ii]].size();
          if (duration > 500)
          {
            progressbar->check_interrupt();
            t_start = std::chrono::high_resolution_clock::now();
          }
        }
        progressbar->end();

        //errorcode = CubitProcessHandler[CubitProcessHandler_data_id].exit_code();
        if (status!=0)
        {
          log = "Job " + jobs_data[jobs_data_id][1] + " with ID " + jobs_data[jobs_data_id][0] + " exited with errors! \n";
          //log.append(" Exit Code " + std::to_string(errorcode) + " \n");
          PRINT_INFO("%s", log.c_str());
          jobs_data[jobs_data_id][3] = "4";
          jobs_data[jobs_data_id][6] = "-1";
        }else{
          log = "Job " + jobs_data[jobs_data_id][1] + " with ID " + jobs_data[jobs_data_id][0] + " finished! \n";
          //log.append(" Exit Code " + std::to_string(errorcode) + " \n");
          PRINT_INFO("%s", log.c_str());
          jobs_data[jobs_data_id][3] = "2";
          results_loaded = complete_job_results(job_id);
        }
        CubitProcessHandler.erase(CubitProcessHandler.begin() + CubitProcessHandler_data_id);
      }
    }
    return results_loaded && get_job_status(job_id) == 2;
  #endif
  return false;
}

bool CoreJobs::kill_job(int job_id)
{ 
  #ifdef WIN32
    std::string log;
    int jobs_data_id=-1;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      HANDLE processhandle;
      DWORD returnCode{};
      processhandle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, std::stoi(jobs_data[jobs_data_id][4]));
      if (GetExitCodeProcess(processhandle, &returnCode)) {
        if (returnCode == STILL_ACTIVE)
        { 
          log = "Kill Job " + jobs_data[jobs_data_id][1] + " with ID " + jobs_data[jobs_data_id][0] + "\n";
          PRINT_INFO("%s", log.c_str());
          const bool terminated = TerminateProcess(processhandle, 1) != FALSE;
          const bool exited = terminated && WaitForSingleObject(processhandle, 1000) == WAIT_OBJECT_0;
          CloseHandle(processhandle);
          if (!exited)
          {
            PRINT_ERROR("Unable to confirm termination of job %d.\n", job_id);
            return false;
          }
          kill_pipe(std::stoi(jobs_data[jobs_data_id][0]));
          log = " Job killed!\n";
          PRINT_INFO("%s", log.c_str());
          jobs_data[jobs_data_id][3] = "3";
          output_console[std::stoi(jobs_data[jobs_data_id][5])].push_back("JOB KILLED!");

          // delete handles
          int ProcessPipe_data_id;
          int PipePID;
          PipePID = get_PipePID_from_ProcessPID(std::stoi(jobs_data[jobs_data_id][4]));           
          ProcessPipe_data_id = get_ProcessPipe_data_id_from_PipePID(PipePID);
          ProcessPipe.erase(ProcessPipe.begin() + ProcessPipe_data_id);
          PPTID.erase(PPTID.begin() + ProcessPipe_data_id);
          this->PipePID.erase(this->PipePID.begin() + ProcessPipe_data_id);
          return true;
        }
      }
      if (processhandle != nullptr)
      {
        CloseHandle(processhandle);
      }
      return false;
    }  
  #else
    std::string log;
    CubitString output;
    
    int jobs_data_id=-1;
    int CubitProcessHandler_data_id;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      CubitProcessHandler_data_id = get_CubitProcessHandler_data_id_from_process_id(std::stoi(jobs_data[jobs_data_id][4]));
      if (CubitProcessHandler_data_id != -1)
      {
        log = " Kill the Job " + jobs_data[jobs_data_id][2] + " \n";
        PRINT_INFO("%s", log.c_str());
        CubitProcessHandler[CubitProcessHandler_data_id].kill();
        CubitProcessHandler[CubitProcessHandler_data_id].wait();
        log = " Job killed with Exit Code " + std::to_string(CubitProcessHandler[CubitProcessHandler_data_id].exit_code()) + " \n";
        PRINT_INFO("%s", log.c_str());
        jobs_data[jobs_data_id][3] = "3";
        CubitProcessHandler.erase(CubitProcessHandler.begin() + CubitProcessHandler_data_id);
      }
    }
    return true;
  #endif
  
  return false;
}

bool CoreJobs::results_ready()
{
  CalculiXResultsInterface results;
  if (completing_results || results.is_vtk_build_active())
  {
    return false;
  }
  for (auto item = pending_result_jobs.begin(); item != pending_result_jobs.end();)
  {
    const int index = get_jobs_data_id_from_job_id(*item);
    if (index < 0 || jobs_data[index][9] != "1" ||
      results.get_vtk_status(*item) == SimResults::ConversionStatus::Complete)
    {
      item = pending_result_jobs.erase(item);
    }
    else
    {
      ++item;
    }
  }
  return pending_result_jobs.empty();
}

bool CoreJobs::complete_job_results(int job_id)
{
  if (completing_results)
  {
    return false;
  }
  QScopedValueRollback<bool> guard(completing_results, true);
  const int index = get_jobs_data_id_from_job_id(job_id);
  if (index < 0)
  {
    return false;
  }
  CalculiXResultsInterface results;
  bool readers_loaded = false;
  try
  {
    jobs_data[index][6] = "-1";
    readers_loaded = results.load_result(job_id);
    if (!readers_loaded)
    {
      jobs_data[index][3] = "4";
      pending_result_jobs.erase(job_id);
      PRINT_ERROR("Failed to load results for job %d.\n", job_id);
      return false;
    }

    // Keep reader data for topology and later manual conversion.
    if (jobs_data[index][9] != "1")
    {
      pending_result_jobs.erase(job_id);
      return true;
    }

    pending_result_jobs.insert(job_id);
    if (!results.build_vtk_result(job_id, CoreResultsVtkOptions{}))
    {
      PRINT_ERROR("VTK conversion failed for job %d. Retry conversion before the next job.\n", job_id);
      return false;
    }
    pending_result_jobs.erase(job_id);
    return true;
  }
  catch (const std::exception& failure)
  {
    if (!readers_loaded)
    {
      jobs_data[index][3] = "4";
      pending_result_jobs.erase(job_id);
    }
    PRINT_ERROR("Unable to prepare results for job %d: %s\n", job_id, failure.what());
    return false;
  }
  catch (...)
  {
    if (!readers_loaded)
    {
      jobs_data[index][3] = "4";
      pending_result_jobs.erase(job_id);
    }
    PRINT_ERROR("Unable to prepare results for job %d.\n", job_id);
    return false;
  }
}

bool CoreJobs::check_jobs()
{
  if (completing_results)
  {
    return false;
  }
  const auto load_finished_results = [&](size_t index)
  {
    complete_job_results(std::stoi(jobs_data[index][0]));
  };

  #ifdef _WIN32
    std::string log;
    CubitString output;
    //int ProcessPipe_data_id;
    //int PipePID;
  
    for (size_t i = 0; i < jobs_data.size(); i++)
    {
      if (std::stoi(jobs_data[i][3])==1)
      {
        const int process_id = std::stoi(jobs_data[i][4]);
        HANDLE processhandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
        DWORD returnCode = STILL_ACTIVE;
        const bool queried = processhandle != nullptr &&
          GetExitCodeProcess(processhandle, &returnCode) != FALSE;
        if (processhandle != nullptr)
        {
          CloseHandle(processhandle);
        }
        if (!queried)
        {
          PRINT_ERROR("Unable to query solver process for job %s.\n", jobs_data[i][0].c_str());
          continue;
        }
        if (returnCode == STILL_ACTIVE)
        {
          continue;
        }
        const auto retained = running_process_handles.find(std::stoi(jobs_data[i][0]));
        if (retained != running_process_handles.end())
        {
          CloseHandle(retained->second);
          running_process_handles.erase(retained);
        }
        const int status = static_cast<int>(returnCode);
        const int pipe_id = get_PipePID_from_ProcessPID(process_id);
        const int pipe_index = get_ProcessPipe_data_id_from_PipePID(pipe_id);
        kill_pipe(std::stoi(jobs_data[i][0]));
        if (pipe_index >= 0 && static_cast<size_t>(pipe_index) < ProcessPipe.size())
        {
          CloseHandle(ProcessPipe[pipe_index]);
          ProcessPipe.erase(ProcessPipe.begin() + pipe_index);
          PPTID.erase(PPTID.begin() + pipe_index);
          PipePID.erase(PipePID.begin() + pipe_index);
        }
        if ((returnCode != STILL_ACTIVE) && (status!=0)) // if process doesn't exist and exited with error
        {
          log = "Job " + jobs_data[i][1] + " with ID " + jobs_data[i][0] + " exited with errors! \n";
          PRINT_INFO("%s", log.c_str());
          jobs_data[i][3] = "4";
          jobs_data[i][6] = "-1";
        }else if ((returnCode != STILL_ACTIVE) && (status==0)) // if process doesn't exist and exited without error
        {
          log = "Job " + jobs_data[i][1] + " with ID " + jobs_data[i][0] + " finished! \n";
          PRINT_INFO("%s", log.c_str());
          jobs_data[i][3] = "2";
          load_finished_results(i);
        }
      }
    }
    return results_ready();
  #else
    std::string log;
    CubitString output;
    int CubitProcessHandler_data_id;
  
    for (size_t i = 0; i < jobs_data.size(); i++)
    {
      // create or get cubitprocess
      CubitProcessHandler_data_id = get_CubitProcessHandler_data_id_from_process_id(std::stoi(jobs_data[i][4]));
          
      if (CubitProcessHandler_data_id != -1)
      {
        /*log = "Job ID " + jobs_data[i][0] + " \n";
        log.append("Job PID " + jobs_data[i][4] + " \n");
        log.append("PID " + std::to_string(CubitProcessHandler[CubitProcessHandler_data_id].pid()) + " \n");
        log.append("CubitProcessHandler_data_id " + std::to_string(CubitProcessHandler_data_id) + " \n");
        PRINT_INFO("%s", log.c_str());*/

        if (std::stoi(jobs_data[i][3])==1)
        {
          // check for output
          int ic=0;
          while (CubitProcessHandler[CubitProcessHandler_data_id].can_read_output())
          { 
            output = CubitProcessHandler[CubitProcessHandler_data_id].read_output_channel(1);
            if ((output != ""))
            {
              log = " Output " + output.str() + " \n";
              //PRINT_INFO("%s", log.c_str());
              //jobs_data[i][5] = jobs_data[i][5] + output.str();
              output_console[std::stoi(jobs_data[i][5])].push_back(output.str());
              output = "";
              get_cvgsta(std::stoi(jobs_data[i][0]));
            }else{
              ic+=1;
            }
            
            // break out of loop, so that output reading doesn't freeze gui
            if (ic==100)
            {
              break;
            }          
          }
          //solver processes still running?
          int status = 0;
          const pid_t finished = waitpid(std::stoi(jobs_data[i][4]), &status, WNOHANG);
          if (finished == 0 || (finished < 0 && errno == EINTR))
          {
            continue;
          }
          if (finished < 0)
          {
            PRINT_ERROR("Unable to query solver process for job %s.\n", jobs_data[i][0].c_str());
            jobs_data[i][3] = "4";
            jobs_data[i][6] = "-1";
            CubitProcessHandler.erase(CubitProcessHandler.begin() + CubitProcessHandler_data_id);
            continue;
          }
          const bool succeeded = WIFEXITED(status) && WEXITSTATUS(status) == 0;
          //log = " wait Code " + std::to_string(status) + " \n";
          //PRINT_INFO("%s", log.c_str());
          
          if (!succeeded) // if process doesn't exist and exited with error
          {
            CubitProcessHandler[CubitProcessHandler_data_id].wait();
            log = "Job " + jobs_data[i][1] + " with ID " + jobs_data[i][0] + " exited with errors! \n";
            //log.append(" Exit Code " + std::to_string(CubitProcessHandler[CubitProcessHandler_data_id].exit_code()) + " \n");
            PRINT_INFO("%s", log.c_str());
            jobs_data[i][3] = "4";
            jobs_data[i][6] = "-1";
            CubitProcessHandler.erase(CubitProcessHandler.begin() + CubitProcessHandler_data_id);
          }else if (succeeded) // if process doesn't exist and exited without error
          {
            CubitProcessHandler[CubitProcessHandler_data_id].wait();
            log = "Job " + jobs_data[i][1] + " with ID " + jobs_data[i][0] + " finished! \n";
            //log.append(" Exit Code " + std::to_string(CubitProcessHandler[CubitProcessHandler_data_id].exit_code()) + " \n");
            PRINT_INFO("%s", log.c_str());
            jobs_data[i][3] = "2";
            load_finished_results(i);
            CubitProcessHandler.erase(CubitProcessHandler.begin() + CubitProcessHandler_data_id);
          }
        }
      }
    }
    return results_ready();
  #endif
  
  return false;
}



int CoreJobs::get_jobs_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    if (jobs_data[i][0]==std::to_string(job_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

#ifdef _WIN32
  int CoreJobs::get_ProcessPipe_data_id_from_PipePID(int PipePID)
  { 
    int return_int = -1;
    for (size_t i = 0; i < this->PipePID.size(); i++)
    {
      /*PULONG ClientProcessId;
      GetNamedPipeClientProcessId(ProcessPipe[i],ClientProcessId);
      
      if (PipePID == int(ClientProcessId))
      {
        return_int = int(i);
      }
      */
      
      if (PipePID == this->PipePID[i])
      {
        return_int = int(i);
      }
    }
    return return_int;
  }

  int CoreJobs::get_PipePID_from_ProcessPID(int ProcessPID)
  {
    int return_int = -1;
    for (size_t i = 0; i < PPTID.size(); i++)
    {
      if (ProcessPID == PPTID[i][0])
      {
        return_int = PPTID[i][1];
      }
    }
    return return_int;
  }

  int CoreJobs::get_ThreadID_from_ProcessPID(int ProcessPID)
  {
    int return_int = -1;
    for (size_t i = 0; i < PPTID.size(); i++)
    {
      if (ProcessPID == PPTID[i][0])
      {
        return_int = PPTID[i][2];
      }
    }
    return return_int;
  }

  void CoreJobs::read_pipe(int job_id, int bool_data_id)
  {
    std::string log;
    CubitString output;
    
    int jobs_data_id=-1;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      HANDLE processhandle;
      DWORD returnCode{};
      processhandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, std::stoi(jobs_data[jobs_data_id][4]));
      if (GetExitCodeProcess(processhandle, &returnCode)) {
        if (returnCode == STILL_ACTIVE)
        { 
          DWORD dwRead; 
          CHAR chBuf[BUFSIZE];
          bool success = true; 
          int ProcessPipe_data_id;
          int PipePID;
          PipePID = get_PipePID_from_ProcessPID(std::stoi(jobs_data[jobs_data_id][4]));           
          ProcessPipe_data_id = get_ProcessPipe_data_id_from_PipePID(PipePID);
          bool_data_id = int(PipeThreadsRun.size()-1);

          while (success) 
          {
            /*
            log = "job_data_id " + std::to_string(jobs_data_id) + "\n";
            PRINT_INFO("%s", log.c_str());
            log = "bool_data_id " + std::to_string(bool_data_id) + "\n";
            PRINT_INFO("%s", log.c_str());
            log = "PipeThreadsRun " + std::to_string(PipeThreadsRun.size()) + "\n";
            PRINT_INFO("%s", log.c_str());
            log = "ProcessPipe_data_id " + std::to_string(ProcessPipe_data_id) + "\n";
            PRINT_INFO("%s", log.c_str());
            */
            if (!PipeThreadsRun[bool_data_id])
            {
              break;
            }
            success = ReadFile(ProcessPipe[ProcessPipe_data_id], chBuf, BUFSIZE, &dwRead, NULL);
            if (!success) {
                break;
            }
            std::string output(chBuf, dwRead);
            output_console[std::stoi(jobs_data[jobs_data_id][5])].push_back(output);
            get_cvgsta(std::stoi(jobs_data[jobs_data_id][0]));
          }
        }
      }
    }
  }

  void CoreJobs::kill_pipe(int job_id)
  {
    std::string log;
    CubitString output;
    int killed_data_id = -1;
    
    int jobs_data_id=-1;
    
    jobs_data_id = get_jobs_data_id_from_job_id(job_id);

    if (jobs_data_id != -1)
    {
      int ThreadID = get_ThreadID_from_ProcessPID(std::stoi(jobs_data[jobs_data_id][4]));   
      if (ThreadID != -1)
      {
        for (size_t i = 0; i < PipeThreads.size(); i++)
        {
          std::ostringstream oss;
          oss << PipeThreads[i].get_id() << std::endl;
          if (ThreadID == std::stoi(oss.str()))
          {
            PipeThreadsRun[i] = false;
            killed_data_id = int(i);
          }
        }
        if (killed_data_id != -1)
        {
          /*
          log = "kill Thread " + std::to_string(ThreadID) + " \n";
          log.append("NUMBER OF PIPE READ THREADS " + std::to_string(PipeThreads.size())  + " \n");
          log.append("NUMBER OF PIPE READ THREADS BOOLS" + std::to_string(PipeThreadsRun.size())  + " \n");
          log.append("kill Thread " + std::to_string(killed_data_id) + " \n");
          */
          PipeThreads[killed_data_id].join();
          PipeThreads.erase(PipeThreads.begin() + killed_data_id);
          PipeThreadsRun.erase(PipeThreadsRun.begin() + killed_data_id);
          /*
          log.append("NUMBER OF PIPE READ THREADS " + std::to_string(PipeThreads.size())  + " \n");
          log.append("NUMBER OF PIPE READ THREADS BOOLS" + std::to_string(PipeThreadsRun.size())  + " \n");
          PRINT_INFO("%s", log.c_str());
          */
        }
      }
    }
  }

#else
  int CoreJobs::get_CubitProcessHandler_data_id_from_process_id(int process_id)
  { 
    int return_int = -1;
    
    for (size_t i = 0; i < CubitProcessHandler.size(); i++)
    {
      if (process_id == CubitProcessHandler[i].pid())
      {
        return_int = int(i);
      }  
    }
    
    return return_int;
  }
#endif

std::vector<std::string> CoreJobs::get_job_data(int job_id)
{
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    return jobs_data[job_data_id];
  }else{
    std::vector<std::string> tmp;
    return tmp;
  }
}

std::vector<std::string> CoreJobs::get_job_console_output(int job_id)
{
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    return output_console[std::stoi(jobs_data[job_data_id][5])];
  }else{
    std::vector<std::string> tmp;
    return tmp;
  }
}

std::vector<std::string> CoreJobs::get_job_cvg(int job_id)
{
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    return cvg[std::stoi(jobs_data[job_data_id][7])];
  }else{
    std::vector<std::string> tmp;
    return tmp;
  }
}

std::vector<std::string> CoreJobs::get_job_sta(int job_id)
{
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    return sta[std::stoi(jobs_data[job_data_id][8])];
  }else{
    std::vector<std::string> tmp;
    return tmp;
  }
}

int CoreJobs::get_job_status(int job_id)
{
  int job_data_id;
  int job_status = -2;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    job_status = std::stoi(jobs_data[job_data_id][3]);
  }
  return job_status;
}

bool CoreJobs::set_job_conversion(int job_id, int conversion)
{
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);
  if (job_data_id != -1)
  {
    jobs_data[job_data_id][6] = std::to_string(conversion);
    return true;
  }else{
    return false;
  }
  return false;
}

bool CoreJobs::is_whitespace(std::string line)
{
  size_t strBegin = line.find_first_not_of(" \t\r\f\v\n");
  if (strBegin == std::string::npos)
  {
    return true;
  }
  return false;
}

std::string CoreJobs::print_data()
{
  std::string str_return;
  str_return = "\n CoreJobs jobs_data: \n";
  str_return.append("job_id, name, filepath, status, process id, output console id, VTK converted, cvg id, sta id \n");

  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    str_return.append(jobs_data[i][0] + " " + jobs_data[i][1] + " " + jobs_data[i][2] + " " + jobs_data[i][3] + " " + jobs_data[i][4] + " " + jobs_data[i][5] + " " + jobs_data[i][6] + " " + jobs_data[i][7] + " " + jobs_data[i][8] + " \n");
    for (size_t ii = 0; ii < output_console[std::stoi(jobs_data[i][5])].size(); ii++)
    {
      str_return.append(output_console[std::stoi(jobs_data[i][5])][ii] + " \n");
    }
    for (size_t ii = 0; ii < cvg[std::stoi(jobs_data[i][7])].size(); ii++)
    {
      str_return.append(cvg[std::stoi(jobs_data[i][7])][ii] + " \n");
    }
    for (size_t ii = 0; ii < sta[std::stoi(jobs_data[i][8])].size(); ii++)
    {
      str_return.append(sta[std::stoi(jobs_data[i][8])][ii] + " \n");
    }
  }

  return str_return;
}

std::vector<std::vector<std::string>> CoreJobs::get_tree_data() const
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& job : jobs_data)
  {
    tree_data.push_back({job[0], job[1]});
  }

  return tree_data;
}

std::vector<int> CoreJobs::get_job_ids() const
{
  return CoreIdSelection::collect_ids(jobs_data);
}


bool CoreJobs::get_settings(int id, JobsSettings& settings, std::string& error) const
{
  settings = {};
  error.clear();
  const auto* record = SettingsRead::row(jobs_data, id, 3, error);

  if (record == nullptr)
  {
    return false;
  }

  settings.id = id;
  settings.name = SettingsRead::text((*record)[1]);
  settings.filepath = SettingsRead::text((*record)[2]);

  if (!error.empty())
  {
    settings = {};
    return false;
  }

  return true;
}
