//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "volta.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      rypos() = 0.0;
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(ST_voltaY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant VoltaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_VOLTA_TYPE:
                  return volta()->getProperty(id);
            default:
                  return TextLineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VoltaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_VOLTA_TYPE:
                  return volta()->setProperty(id, v);
            default:
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VoltaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_VOLTA_TYPE:
                  return volta()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      _voltaType = VoltaType::OPEN;
      setBeginText("1.", s->textStyle(TEXT_STYLE_VOLTA));

      setBeginTextPlace(PLACE_BELOW);
      setContinueTextPlace(PLACE_BELOW);

      setBeginHook(true);
      Spatium hook(s->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setEndHookHeight(hook);
      setAnchor(ANCHOR_MEASURE);
      }

//---------------------------------------------------------
//   setVoltaType
//---------------------------------------------------------

void Volta::setVoltaType(VoltaType val)
      {
      _voltaType = val;
      switch (val) {
            case VoltaType::OPEN:
                  setEndHook(false);
                  break;
            case VoltaType::CLOSED:
                  setEndHook(true);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout()
      {
      setLineWidth(score()->styleS(ST_voltaLineWidth));
      Spatium hook(score()->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setEndHookHeight(hook);
      TextLine::layout();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s, score()->textStyle(TEXT_STYLE_VOLTA));
      foreach(SpannerSegment* seg, spannerSegments())
            static_cast<VoltaSegment*>(seg)->clearText();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText()->text();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setVoltaType(VoltaType(e.readInt()));
            else if (tag == "text")            // obsolete
                  setText(e.readElementText());
            else if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      Volta proto(score());
      proto.setVoltaType(voltaType());

      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", int(_voltaType));
      TextLine::writeProperties(xml, &proto);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      foreach(int ending, endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Volta::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  return int(voltaType());
            default:
                  break;
            }
      return TextLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(P_ID propertyId, const QVariant& val)
      {
      score()->addRefresh(pageBoundingRect());
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  setVoltaType(VoltaType(val.toInt()));
                  break;
            default:
                  if (!TextLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      layout();
      score()->addRefresh(pageBoundingRect());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Volta::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_VOLTA_TYPE:
                  return 0;
            default:
                  return TextLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetVoltaType
//---------------------------------------------------------

void Volta::undoSetVoltaType(VoltaType val)
      {
      score()->undoChangeProperty(this, P_VOLTA_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Volta::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_voltaY).val()) * spatium();
      }

}

