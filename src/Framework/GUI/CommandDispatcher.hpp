#ifndef FRAMEWORKCOMMANDDISPATCHER_HPP
#define FRAMEWORKCOMMANDDISPATCHER_HPP

#include <QString>
#include <string>

class Claro;

namespace CommandDispatcher
{
bool submit(Claro* host, const QString& command, std::string& error);
// Execute without adding a command to the journal/history. Never change Cubit's
// global journaling setting; the caller owns its action-category policy.
bool silent(Claro* host, const QString& command, std::string& error);
}

#endif // FRAMEWORKCOMMANDDISPATCHER_HPP
