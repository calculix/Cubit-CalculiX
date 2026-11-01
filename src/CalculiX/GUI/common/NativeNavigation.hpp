#ifndef CALCULIX_NATIVE_NAVIGATION_HPP
#define CALCULIX_NATIVE_NAVIGATION_HPP

class NavigationModel;
class NavigationNode;

namespace NativeNavigation
{
void copy_branch(NavigationModel* sourceModel, NavigationNode* source, NavigationModel* targetModel, NavigationNode* target);
}

#endif // CALCULIX_NATIVE_NAVIGATION_HPP
