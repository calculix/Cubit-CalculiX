#ifndef POSTPROCESSOR_ANNOTATIONCODEC_HPP
#define POSTPROCESSOR_ANNOTATIONCODEC_HPP
#include "AnnotationDefinition.hpp"
#include <QJsonObject>
namespace Postprocessor
{
  QJsonObject annotations_to_json(const AnnotationState& state);
  bool annotations_from_json(const QJsonValue& value, AnnotationState& state);
}
#endif
