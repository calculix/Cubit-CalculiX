#include "UnitLabelFormat.hpp"
#include <QRegularExpression>
#include <QStringList>
#include <cmath>

namespace
{
QString superscript(const QString& text)
{
  const QString source = QStringLiteral("0123456789-+()mn");
  const QString target = QStringLiteral("\u2070\u00b9\u00b2\u00b3\u2074\u2075\u2076\u2077\u2078\u2079\u207b\u207a\u207d\u207e\u1d50\u207f");
  QString result;
  for (const QChar character : text)
  {
    const auto index = source.indexOf(character);
    if (index < 0)
    {
      return {};
    }
    result += target.at(index);
  }
  return result;
}

QString symbol(const QString& text)
{
  if (text == QStringLiteral("degC"))
  {
    return QStringLiteral("\u00b0C");
  }
  if (text == QStringLiteral("degF"))
  {
    return QStringLiteral("\u00b0F");
  }
  if (text == QStringLiteral("deg"))
  {
    return QStringLiteral("\u00b0");
  }
  if (text == QStringLiteral("delta_degC"))
  {
    return QStringLiteral("\u0394\u00b0C");
  }
  if (text == QStringLiteral("delta_degF"))
  {
    return QStringLiteral("\u0394\u00b0F");
  }
  return text;
}

QString integer_power(const QString& base, int exponent)
{
  if (exponent == 1)
  {
    return base;
  }
  return base + superscript(QString::number(exponent));
}

QString product(QStringList factors, const QStringList& roots)
{
  if (!roots.isEmpty())
  {
    const QString radicand = roots.join(QStringLiteral("\u00b7"));
    if (roots.size() == 1)
    {
      factors.push_back(QStringLiteral("\u221a") + radicand);
    }
    else
    {
      factors.push_back(QStringLiteral("\u221a(") + radicand + QStringLiteral(")"));
    }
  }
  if (factors.isEmpty())
  {
    return QStringLiteral("1");
  }
  return factors.join(QStringLiteral("\u00b7"));
}
}

QString UnitLabelFormat::format(const std::string& expression)
{
  const QString original = QString::fromStdString(expression);
  const QString text = original.trimmed();
  if (text.isEmpty())
  {
    return original;
  }
  static const QRegularExpression token(QStringLiteral(R"(\s*([*/]?)\s*([\p{L}_%][\p{L}\p{N}_%]*|1)\s*(?:\^\s*([+-]?[0-9]+(?:\.[0-9]+)?))?\s*)"));
  QStringList numerator;
  QStringList denominator;
  QStringList numeratorRoots;
  QStringList denominatorRoots;
  qsizetype position = 0;
  while (position < text.size())
  {
    const auto match = token.match(text, position);
    if (!match.hasMatch() || match.capturedStart() != position ||
        (position == 0 && !match.captured(1).isEmpty()) ||
        (position != 0 && match.captured(1).isEmpty()))
    {
      return original;
    }
    double exponent = 1.0;
    if (!match.captured(3).isEmpty())
    {
      bool valid = false;
      exponent = match.captured(3).toDouble(&valid);
      if (!valid || !std::isfinite(exponent) || std::abs(exponent) > 1000.0)
      {
        return original;
      }
    }
    if (match.captured(1) == QStringLiteral("/"))
    {
      exponent = -exponent;
    }
    const double magnitude = std::abs(exponent);
    // Unit symbols represent positive scale factors. Consequently half powers
    // can share a radical. Keep arbitrary fractional powers unchanged.
    if (2.0 * magnitude != std::floor(2.0 * magnitude))
    {
      return original;
    }
    if (exponent != 0.0 && match.captured(2) != QStringLiteral("1"))
    {
      const QString base = symbol(match.captured(2));
      if (magnitude == std::floor(magnitude))
      {
        if (exponent > 0.0)
        {
          numerator.push_back(integer_power(base, static_cast<int>(magnitude)));
        }
        else
        {
          denominator.push_back(integer_power(base, static_cast<int>(magnitude)));
        }
      }
      else if (exponent > 0.0)
      {
        numeratorRoots.push_back(integer_power(base, static_cast<int>(2.0 * magnitude)));
      }
      else
      {
        denominatorRoots.push_back(integer_power(base, static_cast<int>(2.0 * magnitude)));
      }
    }
    position = match.capturedEnd();
  }
  const QString top = product(numerator, numeratorRoots);
  if (denominator.isEmpty() && denominatorRoots.isEmpty())
  {
    return top;
  }
  QString bottom = product(denominator, denominatorRoots);
  qsizetype factors = denominator.size();
  if (!denominatorRoots.isEmpty())
  {
    ++factors;
  }
  if (factors > 1)
  {
    bottom = QStringLiteral("(") + bottom + QStringLiteral(")");
  }
  return top + QStringLiteral("/") + bottom;
}

QString UnitLabelFormat::power(const std::string& expression, const QString& exponent)
{
  const QString raised = superscript(exponent);
  if (raised.isEmpty())
  {
    return QStringLiteral("(") + QString::fromStdString(expression) + QStringLiteral(")^(") + exponent + QStringLiteral(")");
  }
  return QStringLiteral("(") + format(expression) + QStringLiteral(")") + raised;
}
