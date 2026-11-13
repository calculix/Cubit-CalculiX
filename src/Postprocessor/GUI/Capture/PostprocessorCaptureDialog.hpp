#pragma once
#include <QString>
class QWidget;
class PostprocessorApplicationInterface;
// Returns a journalable command after acceptance, or an empty string on cancel.
QString postprocessor_capture_dialog(QWidget* parent,const PostprocessorApplicationInterface& application,bool report);
