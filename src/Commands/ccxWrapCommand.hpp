#ifndef CCXWRAPCOMMAND_HPP
#define CCXWRAPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxWrapCommand : public CubitCommand
{
public:
    std::vector<std::string> get_syntax() override;
    std::vector<std::string> get_syntax_help() override;
    std::vector<std::string> get_help() override;
    bool execute(CubitCommandData& data) override;
};

#endif