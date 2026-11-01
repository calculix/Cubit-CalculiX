#ifndef CORESIDESETFACES_HPP
#define CORESIDESETFACES_HPP

#include <string>
#include <vector>

class CoreSidesetFaces
{
public:
  bool add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type);
  std::vector<std::vector<std::string>> get_sideset_face(int sideset_id) const;
  void clear();

private:
  std::vector<std::vector<std::string>> faces;
};

#endif
