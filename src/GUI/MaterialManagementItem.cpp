
#include "MaterialManagementItem.hpp"
#include "CalculiXCoreInterface.hpp"

MaterialManagementItem::MaterialManagementItem(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

MaterialManagementItem::~MaterialManagementItem()
{
}

void MaterialManagementItem::initialize(QString material_id_qstring,QString material_name)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  QTreeWidgetItem::setText(0, material_name);
  QTreeWidgetItem::setText(1, material_id_qstring);
  
  isInitialized = true;
}

void MaterialManagementItem::update()
{
}