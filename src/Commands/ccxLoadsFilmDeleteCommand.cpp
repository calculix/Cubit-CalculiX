#include "ccxLoadsFilmDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsFilmDeleteCommand::ccxLoadsFilmDeleteCommand()
{}

ccxLoadsFilmDeleteCommand::~ccxLoadsFilmDeleteCommand()
{}

std::vector<std::string> ccxLoadsFilmDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete film ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='film id',help='<film id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='film id',help='<film id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <film id>...
      syntax.append("all except ");
      syntax.append("<value:label='film id except'>... ");
    }else if (syn_i==4)
    {
      // all except <film id> to <film id 2>
      syntax.append("all except ");
      syntax.append("<value:label='film id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='film id s2'>");
    }else if (syn_i==5)
    {
      // <film id> to <film id 2>
      syntax.append("<value:label='film id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='film id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsFilmDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete film <film id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsFilmDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsFilmDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> film_ids;
  int film_id_s1;
  int film_id_s2;
  std::vector<std::string> films_string;

  std::string film_string = " ";
  
  data.get_value("film id s1", film_id_s1);
  data.get_value("film id s2", film_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("film id except", film_ids))
    {
      film_string.append("all except");
      for (size_t i = 0; i < film_ids.size(); i++)
      {
        film_string.append(" " + std::to_string(film_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    film_string.append("all except " + std::to_string(film_id_s1) + " to " + std::to_string(film_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    film_string.append(std::to_string(film_id_s1) + " to " + std::to_string(film_id_s2));
  }else if(data.get_strings("film id", films_string))
  {
    for (size_t i = 0; i < films_string.size(); i++)
    {
      film_string.append(films_string[i]);
    }
  }
  
  if(!data.get_values("film id", film_ids))
  {   
    film_ids = ccx_iface.parser("loadsfilm", film_string);
  }

  if (film_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < film_ids.size(); i++)
  {    
    if (!ccx_iface.delete_loadsfilm(film_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(film_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}