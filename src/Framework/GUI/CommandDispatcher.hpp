#ifndef FRAMEWORKCOMMANDDISPATCHER_HPP
#define FRAMEWORKCOMMANDDISPATCHER_HPP

#include <QString>
#include <string>

class Claro;

namespace CommandDispatcher
{
bool submit(Claro* host, const QString& command, std::string& error);
}

#endif // FRAMEWORKCOMMANDDISPATCHER_HPP
