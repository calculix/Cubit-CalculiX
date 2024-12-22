#ifndef CCXEXPORTCOMMAND_HPP
#define CCXEXPORTCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class MeshExportInterface;
class MaterialInterface;
class CalculiXCoreInterface;
class ProgressTool;

/*!
 * \brief The ccxExportCommand class implements a calculix export command.
 */
class ccxExportCommand: public CubitCommand
{
public:
  ccxExportCommand();
  ~ccxExportCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);

protected:
  //! \brief open a file
  //! \param[in] file the name of the file to opened
  //! \param[out] output_file the file stream that is used for writing
  //! \return true if the file open succeeded
  bool open_file(const std::string &file, std::ofstream& output_file);

  //! \brief write data to the file
  //! \param output_file the file stream that is used for writing
  //! \param[in] iface the reference to the MeshExportInterface
  //! \return true if file write succeeded
  bool write_file(std::ofstream& output_file, MeshExportInterface *iface, MaterialInterface *material_iface, CalculiXCoreInterface ccx_iface);

  //! \brief close the file
  //! \param output_file the file stream that is used for writing
  //! \returns always returns true
  bool close_file(std::ofstream& output_file);

  //! \brief method used internally to write_file to write the mesh connectivity
  //! \param output_file the file stream that is used for writing
  //! \param[in] iface the reference to the MeshExportInterface
  //! \return true if file write succeeded
  bool write_connectivity(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface);

  bool write_nodes(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface);
  //bool write_blocks(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface);
  bool write_nodesets(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface);
  bool write_sidesets(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface);
  bool write_materials(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_sections(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_constraints(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_surfaceinteractions(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_contactpairs(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_amplitudes(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_orientations(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_damping(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_physicalconstants(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_initialconditions(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_hbcs(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);
  bool write_steps(std::ofstream& output_file,CalculiXCoreInterface ccx_iface);

  ProgressTool *progressbar; // progressbar
};

#endif // CCXEXPORTCOMMAND_HPP
