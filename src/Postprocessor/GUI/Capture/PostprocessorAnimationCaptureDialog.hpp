#pragma once
#include <QString>
class QWidget;
class PostprocessorApplicationInterface;
QString postprocessor_animation_capture_dialog(QWidget* parent,const PostprocessorApplicationInterface& application);
