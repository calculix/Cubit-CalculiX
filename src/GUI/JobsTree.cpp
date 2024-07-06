#include "JobsTree.hpp"
#include "CalculiXCoreInterface.hpp"
#include "GUITimer.hpp"

JobsTree::JobsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

JobsTree::~JobsTree()
{
}

void JobsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Jobs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("JobsTree")); 
  isInitialized = true;
  timer = new GUITimer();
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update_icon()));
}

void JobsTree::update()
{
  std::vector<std::vector<std::string>> jobs_tree_data;
  jobs_tree_data = ccx_iface->get_jobs_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < jobs_tree_data.size(); i++)
  {
    // check if job already exists as item
    
    ChildId = this->get_child_id(jobs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addJob(QString::fromStdString(jobs_tree_data[i][0]),
        QString::fromStdString(jobs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = jobs_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = this->childCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = this->child(int(i)-1);

    for (size_t ii = 0; ii < jobs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==jobs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeJob(temp_child);
    }
  }
}

void JobsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void JobsTree::addJob(QString job_id, QString job_name)
{
  QTreeWidgetItem *JobTreeChild = new QTreeWidgetItem();
  
  JobTreeChild->setText(0, job_name);
    
  JobTreeChild->setText(1, job_id);

  JobTreeChild->setIcon(0,ccx_iface->getIcon2("JobsTree")); 

  this->addChild(JobTreeChild);
}

void JobsTree::removeJob(QTreeWidgetItem *job)
{
  this->removeChild(job);
  delete job;
}

int JobsTree::get_child_id(std::string job_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    temp_child = this->child(int(i));
    if (temp_child->text(1).toStdString()==job_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void JobsTree::update_icon()
{
  std::vector<std::vector<std::string>> jobs_tree_data;
  std::vector<std::string> job_data;
  jobs_tree_data = ccx_iface->get_jobs_tree_data();
  
  QTreeWidgetItem *temp_child;
  int ChildId;

  for (size_t i = 0; i < jobs_tree_data.size(); i++)
  {
    // check if job already exists as item
    
    ChildId = this->get_child_id(jobs_tree_data[i][0]);

    if (ChildId != -1)
    {
      temp_child = this->child(ChildId);
      job_data = ccx_iface->get_job_data(std::stoi(jobs_tree_data[i][0]));    

      // jobs_data[0][3] status -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors
      if (job_data[3] == "-1")
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("JobsTree_no_process"));
      }else if (job_data[3] == "1")
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("JobsTree_running"));
      }else if (job_data[3] == "2")
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("JobsTree_finished"));
      }else if (job_data[3] == "3")
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("JobsTree_killed"));
      }else if (job_data[3] == "4")
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("JobsTree_error"));
      }
    }
  }
}