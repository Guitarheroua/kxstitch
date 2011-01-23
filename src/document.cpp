/****************************************************************************
 *	Copyright (C) 2010 by Stephen Allewell									*
 *	stephen@mirramar.adsl24.co.uk											*
 *																			*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or		*
 *	(at your option) any later version.										*
 ****************************************************************************/


#include <QFile>
#include <QtGlobal>

#include <KMessageBox>
#include <KIO/NetAccess>

#include "backgroundimage.h"
#include "configuration.h"
#include "document.h"
#include "flossscheme.h"
#include "palettemanagerdlg.h"
#include "schememanager.h"


Document::Document()
{
	m_schemeManager = new SchemeManager();

	initialiseNew();
}


Document::~Document()
{
	qDeleteAll(m_backgroundImages);
	qDeleteAll(m_palette);
	qDeleteAll(m_canvasStitches);
	qDeleteAll(m_canvasKnots);
	qDeleteAll(m_canvasBackstitches);
}


void Document::initialiseNew()
{
	// set all the default properties to display a new document
	// these will be updated by configuring or loading a document

	m_properties["width"] = Configuration::document_Width();
	m_properties["height"] = Configuration::document_Height();
	m_properties["title"] = QString();
	m_properties["author"] = QString();
	m_properties["copyright"] = QString();
	m_properties["fabric"] = QString();
	m_properties["fabricColor"] = QColor();
	m_properties["instructions"] = QString();
	m_properties["flossSchemeName"] = Configuration::palette_DefaultScheme();
	m_properties["currentFlossIndex"] = -1;
	m_properties["cellWidth"] = Configuration::editor_CellWidth();
	m_properties["cellHeight"] = Configuration::editor_CellHeight();
	m_properties["cellHorizontalGrouping"] = Configuration::editor_HorizontalCellGrouping();
	m_properties["cellVerticalGrouping"] = Configuration::editor_VerticalCellGrouping();
	m_properties["horizontalClothCount"] = Configuration::editor_HorizontalClothCount();
	m_properties["verticalClothCount"] = Configuration::editor_VerticalClothCount();
	m_properties["clothCountUnits"] = Configuration::editor_ClothCountUnits();
	m_properties["thickLineWidth"] = Configuration::editor_ThickLineWidth();
	m_properties["thinLineWidth"] = Configuration::editor_ThinLineWidth();
	m_properties["thickLineColor"] = Configuration::editor_ThickLineColor();
	m_properties["thinLineColor"] = Configuration::editor_ThinLineColor();
	m_properties["formatScalesAs"] = Configuration::editor_FormatScalesAs();
	m_properties["showStitchesAs"] = Configuration::editor_ShowStitchesAs();
	m_properties["showBackstitchesAs"] = Configuration::editor_ShowBackstitchesAs();
	m_properties["showKnotsAs"] = Configuration::editor_ShowKnotsAs();
	m_properties["showPaletteSymbols"] = Configuration::palette_ShowSymbols();
	m_properties["paintBackgroundImages"] = true; // Configuration::editor_PaintBackgroundImages();
	m_properties["paintGrid"] = true; // Configuration::editor_PaintGrid();
	m_properties["paintStitches"] = true; // Configuration::editor_PaintStitches();
	m_properties["paintBackstitches"] = true; // Configuration::editor_PaintBackstitches;
	m_properties["paintFrenchKnots"] = true; // Configuration::editor_PaintFrenchKnots;

	// use qDeleteAll to delete the objects pointed to in the containers
	// then clear the containers.
	qDeleteAll(m_backgroundImages);
	m_backgroundImages.clear();

	qDeleteAll(m_palette);
	m_palette.clear();

	qDeleteAll(m_canvasStitches);
	m_canvasStitches.clear();

	m_usedFlosses.clear();	// no requirement for qDeleteAll as m_usedFlosses does not contain pointers.

	qDeleteAll(m_canvasKnots);
	m_canvasKnots.clear();

	qDeleteAll(m_canvasBackstitches);
	m_canvasBackstitches.clear();

	setURL(i18n("Untitled"));
}


bool Document::loadURL(const KUrl &url)
{
	QString tmpfile;
	bool	validRead = false;

	QString	magic;
	quint16 fileFormatVersion;
	quint32 width;
	quint32 height;
	QString title;
	QString author;
	QString copyright;
	QString fabric;
	QColor fabricColor;
	QString instructions;
	QString flossSchemeName;
	quint32 paletteCount;
	quint32 flossKey;
	FlossScheme *flossScheme;
	QString flossName;
	QChar	flossSymbol;
	quint8	stitchStrands;
	quint8	backstitchStrands;
	qint32	currentFlossIndex;
	quint32 usedFlosses;
	quint32 usedFlossesValue;
	quint32	canvasStitches;
	quint32	canvasStitchKey;
	quint8	stitchQueueCount;
	quint8	stitchType;
	quint32 usedFlossKey;
	quint32 usedFlossValue;
	quint32 canvasBackstitches;
	QPoint	start;
	QPoint	end;
	quint32 canvasKnots;
	quint32	backgroundImages;
	KUrl	imageURL;
	QRect	imageLocation;
	bool	imageVisible;
	QImage	image;
	QIcon	imageIcon;

	quint32 count;

	if (!url.isEmpty())
	{
		if (KIO::NetAccess::download(url, tmpfile, 0))
		{
			QFile file(tmpfile);
			if (file.open(QIODevice::ReadOnly))
			{
				QDataStream stream(&file);

				stream >> magic;
				if (file.error()) return false;
				if (magic == "KXStitch")
				{
					stream >> fileFormatVersion;
					if (file.error())
					{
						file.close();
						return false;
					}
					switch (fileFormatVersion)
					{
						case 9:
							stream	>> width
									>> height
									>> title
									>> author
									>> copyright
									>> fabric
									>> fabricColor
									>> instructions
									>> m_properties
									>> flossSchemeName;
							if (file.error()) break;
							m_properties["width"] = width;
							m_properties["height"] = height;
							m_properties["title"] = title;
							m_properties["author"] = author;
							m_properties["copyright"] = copyright;
							m_properties["fabric"] = fabric;
							m_properties["fabricColor"] = fabricColor;
							m_properties["instructions"] = instructions;
							m_properties["flossSchemeName"] = flossSchemeName;

							stream	>> paletteCount;
							if (file.error()) break;
							flossScheme = m_schemeManager->scheme(m_properties["flossSchemeName"].toString());
							while (paletteCount--)
							{
								stream	>> flossKey
										>> flossName
										>> flossSymbol
										>> stitchStrands
										>> backstitchStrands;
								if (file.error()) break;
								Floss *floss = flossScheme->find(flossName);
								DocumentFloss *newFloss = new DocumentFloss(floss, flossSymbol, stitchStrands, backstitchStrands);
								m_palette[flossKey] = newFloss;
							}

							stream >> currentFlossIndex;
							if (file.error()) break;
							m_properties["currentFlossIndex"] = currentFlossIndex;

							stream >> usedFlosses;
							if (file.error()) break;
							while (usedFlosses--)
							{
								stream	>> flossKey
										>> usedFlossesValue;
								if (file.error()) break;
								m_usedFlosses[flossKey] = usedFlossesValue;
							}

							stream >> canvasStitches;
							if (file.error()) break;
							while (canvasStitches--)
							{
								stream	>> canvasStitchKey
										>> stitchQueueCount;
								if (file.error()) break;
								m_canvasStitches[canvasStitchKey] = new StitchQueue;
								while (stitchQueueCount--)
								{
									stream	>> stitchType
											>> flossKey;
									if (file.error()) break;
									m_canvasStitches[canvasStitchKey]->enqueue(new Stitch((Stitch::Type)stitchType, flossKey));
								}
							}

							stream >> canvasBackstitches;
							if (file.error()) break;
							while (canvasBackstitches--)
							{
								stream	>> start
										>> end
										>> flossKey;
								if (file.error()) break;
								m_canvasBackstitches.append(new Backstitch(start, end, flossKey));
							}

							stream >> canvasKnots;
							if (file.error()) return false;
							while (canvasKnots--)
							{
								stream	>> start
										>> flossKey;
								if (file.error()) break;
								m_canvasKnots.append(new Knot(start, flossKey));
							}

							stream >> backgroundImages;
							if (file.error()) break;
							while (backgroundImages--)
							{
								stream	>> imageURL
										>> imageLocation
										>> imageVisible
										>> image
										>> imageIcon;
								if (file.error()) break;
								BackgroundImage *backgroundImage = new BackgroundImage(imageURL, imageLocation, imageVisible, image, imageIcon);
								m_backgroundImages.append(backgroundImage);
							}
							validRead = true;
							break;

						case 10:
							stream >> m_properties;
							if (file.error()) break;

							flossScheme = m_schemeManager->scheme(m_properties["flossSchemeName"].toString());

							stream >> count;
							while (count--)
							{
									stream  >> flossKey
											>> flossName
											>> flossSymbol
											>> stitchStrands
											>> backstitchStrands;
									Floss *floss = flossScheme->find(flossName);
									m_palette[flossKey] = new DocumentFloss(floss, flossSymbol, stitchStrands, backstitchStrands);
							}
							if (file.error()) break;

							stream >> count;
							while (count--)
							{
									stream  >> usedFlossKey
											>> usedFlossValue;
									m_usedFlosses[usedFlossKey] = usedFlossValue;
							}
							if (file.error()) break;

							stream >> count;
							while (count--)
							{
									stream  >> canvasStitchKey
											>> stitchQueueCount;
									m_canvasStitches[canvasStitchKey] = new StitchQueue;
									while (stitchQueueCount--)
									{
											stream  >> stitchType
													>> flossKey;
											m_canvasStitches[canvasStitchKey]->enqueue(new Stitch((Stitch::Type)stitchType, flossKey));
									}
							}
							if (file.error()) break;

							stream >> count;
							while (count--)
							{
									stream  >> start
											>> end
											>> flossKey;
									m_canvasBackstitches.append(new Backstitch(start, end, flossKey));
							}
							if (file.error()) break;

							stream >> count;
							while (count--)
							{
									stream  >> start
											>> flossKey;
									m_canvasKnots.append(new Knot(start, flossKey));
							}
							if (file.error()) break;

							stream >> count;
							while (count--)
							{
									stream  >> imageURL
											>> imageLocation
											>> imageVisible
											>> image
											>> imageIcon;
									m_backgroundImages.append(new BackgroundImage(imageURL, imageLocation, imageVisible, image, imageIcon));
							}
							if (file.error()) break;

							validRead = true;

							break;

						default:
							kDebug() << "Invalid file format encountered.";
							break;
					}

					if (validRead)
					{
						m_undoStack.clear();
						setURL(url);
					}
				}
				else
				{
					KMessageBox::detailedError(0, i18n("This file does not appear to be a KXStitch file."), i18n("Not a KXStitch file."));
				}
				file.close();
			}
		}
	}
	return validRead;
}


unsigned int Document::width() const
{
	return m_properties["width"].toUInt();
}


unsigned int Document::height() const
{
	return m_properties["height"].toUInt();
}


void Document::setURL(const KUrl &url)
{
	if (url.fileName().right(4).toLower() == ".pat")	// this looks like a PCStitch file name
		m_documentURL.setFileName(i18n("Untitled"));	// so that the user doesn't overwrite a PCStitch
														// file with a KXStitch file.
	else
		m_documentURL = url;
}


KUrl Document::URL() const
{
	return m_documentURL;
}


bool Document::isModified() const
{
	return !m_undoStack.isClean();
}


bool Document::saveDocument()
{
	// open the file identified by m_documentURL

	const quint16 FILE_FORMAT_VERSION = 10;

	QFile file(m_documentURL.path());
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream stream(&file);
		stream << QString("KXStitch");
		stream << FILE_FORMAT_VERSION;
		stream << m_properties;

		stream << (quint32)m_palette.count();
		QMap<int, DocumentFloss *>::const_iterator flossIterator;
 		for (flossIterator = m_palette.constBegin() ; flossIterator != m_palette.constEnd() ; ++flossIterator)
			stream	<< (quint32)flossIterator.key()
					<< flossIterator.value()->floss()->name()
					<< flossIterator.value()->symbol()
					<< (quint8)(flossIterator.value()->stitchStrands())
					<< (quint8)(flossIterator.value()->backstitchStrands());

		stream << (quint32)m_usedFlosses.count();
		QMap<int, int>::const_iterator usedFlossesIterator;
		for (usedFlossesIterator = m_usedFlosses.constBegin() ; usedFlossesIterator != m_usedFlosses.constEnd() ; ++usedFlossesIterator)
			stream	<< (quint32)usedFlossesIterator.key()
					<< (quint32)usedFlossesIterator.value();

		stream << (quint32)m_canvasStitches.count();
		QMap<unsigned int, StitchQueue *>::const_iterator stitchIterator;
		for (stitchIterator = m_canvasStitches.constBegin() ; stitchIterator != m_canvasStitches.constEnd() ; ++stitchIterator)
		{
			stream << (quint32)stitchIterator.key();
			StitchQueue *stitchQueue = stitchIterator.value();
			stream << (quint8)stitchQueue->count();
			StitchQueue::const_iterator si;
			for (si = stitchQueue->constBegin() ; si != stitchQueue->constEnd() ; ++si)
				stream	<< (quint8)((*si)->type())
						<< (quint32)((*si)->floss());
		}

		stream << (quint32)m_canvasBackstitches.count();
		QList<Backstitch *>::const_iterator backstitchIterator;
		for (backstitchIterator = m_canvasBackstitches.constBegin() ; backstitchIterator != m_canvasBackstitches.constEnd() ; ++backstitchIterator)
			stream	<< (*backstitchIterator)->start()
					<< (*backstitchIterator)->end()
					<< (quint32)((*backstitchIterator)->floss());

		stream << (quint32)m_canvasKnots.count();
		QList<Knot *>::const_iterator knotIterator;
		for (knotIterator = m_canvasKnots.constBegin() ; knotIterator != m_canvasKnots.constEnd() ; ++knotIterator)
			stream	<< (*knotIterator)->position()
					<< (quint32)((*knotIterator)->floss());

		stream << (quint32)m_backgroundImages.count();
		QList<BackgroundImage *>::const_iterator backgroundImageIterator;
		for (backgroundImageIterator = m_backgroundImages.constBegin() ; backgroundImageIterator != m_backgroundImages.constEnd() ; ++backgroundImageIterator)
			stream	<< (*backgroundImageIterator)->URL()
					<< (*backgroundImageIterator)->location()
					<< (*backgroundImageIterator)->isVisible()
					<< (*backgroundImageIterator)->image()
					<< (*backgroundImageIterator)->icon();

		file.flush();
		file.close();

		m_undoStack.setClean();

		return true;
	}
	return false;
}


QVariant Document::property(const QString &propertyName) const
{
	if (! m_properties.contains(propertyName))
		kDebug() << "Asked for non existant property " << propertyName;
	return m_properties.value(propertyName, QVariant(""));
}


void Document::setProperty(const QString &propertyName, const QVariant &propertyValue)
{
	m_properties[propertyName] = propertyValue;
}


StitchQueue *Document::stitchAt(const QPoint &cell) const
{
	if (validateCell(cell))
		return m_canvasStitches.value(canvasIndex(cell)); // this will be 0 if no queue exists at x,y
	return 0;
}


const Floss *Document::floss(int index) const
{
	return m_palette[index]->floss();
}


int Document::currentFlossIndex() const
{
	return m_properties["currentFlossIndex"].toInt();;
}


void Document::setCurrentFlossIndex(int floss)
{
	m_properties["currentFlossIndex"] = floss;
}


bool Document::addStitch(Stitch::Type type, const QPoint &cell)
{
	if (!validateCell(cell))
		return false;

	unsigned int index = canvasIndex(cell);

	StitchQueue *stitchQueue = m_canvasStitches.value(index);
	if (stitchQueue == 0)
	{
		/** no stitch queue currently exists for this cell */
		stitchQueue = new StitchQueue();
		m_canvasStitches[index] = stitchQueue;
	}

	bool miniStitch = (type & 192);

	unsigned int stitchCount = stitchQueue->count();

	unsigned int stitches = stitchCount;

	if (!miniStitch)
	{
		// try and merge it with any existing stitches in the queue to update the stitch being added
		while (stitches--)
		{
			Stitch *stitch = stitchQueue->dequeue();
			if (!(stitch->type() & 192)) // so we don't try and merge existing mini stitches
			{
				if (stitch->floss() == m_properties["currentFlossIndex"].toInt())
				{
					type = (Stitch::Type)(type | stitch->type());
				}
			}
			stitchQueue->enqueue(stitch);
		}
	}

	switch (type) // add the new stitch checking for illegal types
	{
		case 3: // TLQtr and TRQtr
			stitchQueue->enqueue(new Stitch(Stitch::TLQtr, m_properties["currentFlossIndex"].toInt()));
			stitchQueue->enqueue(new Stitch(Stitch::TRQtr, m_properties["currentFlossIndex"].toInt()));
			m_usedFlosses[m_properties["currentFlossIndex"].toInt()] += 2;
			break;
		case 5: // TLQtr and BLQtr
			stitchQueue->enqueue(new Stitch(Stitch::TLQtr, m_properties["currentFlossIndex"].toInt()));
			stitchQueue->enqueue(new Stitch(Stitch::BLQtr, m_properties["currentFlossIndex"].toInt()));
			m_usedFlosses[m_properties["currentFlossIndex"].toInt()] += 2;
			break;
		case 10: // TRQtr and BRQtr
			stitchQueue->enqueue(new Stitch(Stitch::TRQtr, m_properties["currentFlossIndex"].toInt()));
			stitchQueue->enqueue(new Stitch(Stitch::BRQtr, m_properties["currentFlossIndex"].toInt()));
			m_usedFlosses[m_properties["currentFlossIndex"].toInt()] += 2;
			break;
		case 12: // BLQtr and BRQtr
			stitchQueue->enqueue(new Stitch(Stitch::BLQtr, m_properties["currentFlossIndex"].toInt()));
			stitchQueue->enqueue(new Stitch(Stitch::BRQtr, m_properties["currentFlossIndex"].toInt()));
			m_usedFlosses[m_properties["currentFlossIndex"].toInt()] += 2;
			break;
		default: // other values are acceptable as is including mini stitches
			stitchQueue->enqueue(new Stitch(type, m_properties["currentFlossIndex"].toInt()));
			m_usedFlosses[m_properties["currentFlossIndex"].toInt()]++;
			break;
	}

	/** iterate the queue of existing stitches for any that have been overwritten by the new stitch */
	while (stitchCount--)                                              // while there are existing stitches
	{
		Stitch *stitch = stitchQueue->dequeue();                         // get the stitch at the head of the queue
		m_usedFlosses[stitch->floss()]--;
		Stitch::Type currentStitchType = (Stitch::Type)(stitch->type());   // and find its type
		int currentFlossIndex = stitch->floss();                           // and color
		Stitch::Type usageMask = (Stitch::Type)(currentStitchType & 15); // and find which parts of a stitch cell are used
		Stitch::Type interferenceMask = (Stitch::Type)(usageMask & type);
		// interferenceMask now contains a mask of which bits are affected by new stitch
		if (interferenceMask)
		{
			// Some parts of the current stitch are being overwritten
			// but a check needs to be made for illegal values
			Stitch::Type changeMask = (Stitch::Type)(usageMask ^ interferenceMask);
			switch (changeMask)
			{
				// changeMask contains what is left of the original stitch after being overwritten
				// it may contain illegal values, so these are checked for
				case 3:
					stitchQueue->enqueue(new Stitch(Stitch::TLQtr, currentFlossIndex));
					stitchQueue->enqueue(new Stitch(Stitch::TRQtr, currentFlossIndex));
					m_usedFlosses[currentFlossIndex] += 2;
					changeMask = Stitch::Delete;
					break;
				case 5:
					stitchQueue->enqueue(new Stitch(Stitch::TLQtr, currentFlossIndex));
					stitchQueue->enqueue(new Stitch(Stitch::BLQtr, currentFlossIndex));
					m_usedFlosses[currentFlossIndex] += 2;
					changeMask = Stitch::Delete;
					break;
				case 10:
					stitchQueue->enqueue(new Stitch(Stitch::TRQtr, currentFlossIndex));
					stitchQueue->enqueue(new Stitch(Stitch::BRQtr, currentFlossIndex));
					m_usedFlosses[currentFlossIndex] += 2;
					changeMask = Stitch::Delete;
					break;
				case 12:
					stitchQueue->enqueue(new Stitch(Stitch::BLQtr, currentFlossIndex));
					stitchQueue->enqueue(new Stitch(Stitch::BRQtr, currentFlossIndex));
					m_usedFlosses[currentFlossIndex] += 2;
					changeMask = Stitch::Delete;
					break;
				default:
					// other values are acceptable as is
					break;
			}

			if (changeMask) // Check if there is anything left of the original stitch, Stitch::Delete is 0
			{
				stitch->setType(changeMask);           // and change stitch type to the changeMask value
				stitchQueue->enqueue(stitch);        // and then add it back to the queue
				m_usedFlosses[stitch->floss()]++;
			}
			else // if changeMask is 0, it does not get requeued, effectively deleting it from the pattern
			{
				delete stitch;                     // delete the Stitch as it is no longer required
			}
		}
		else
		{
			stitchQueue->enqueue(stitch);
			m_usedFlosses[stitch->floss()] += 1;
		}
	}

	return true;
}


bool Document::deleteStitch(const QPoint &cell, Stitch::Type maskStitch, int maskColor)
{
	if (!validateCell(cell))
		return false;			// Cell isn't valid so it can't be deleted

	unsigned int index = canvasIndex(cell);

	StitchQueue *stitchQueue = m_canvasStitches.value(index);
	if (stitchQueue == 0)
		return false;			// No stitch queue exists at the required location so nothing to delete

	if (maskStitch == Stitch::Delete)
	{
		int stitchCount = stitchQueue->count();
		while (stitchCount--)
		{
			Stitch *stitch = stitchQueue->dequeue();
			m_usedFlosses[stitch->floss()]--;
			if (!maskColor || (stitch->floss() == m_properties["currentFlossIndex"].toInt()))
			{
				delete stitch;
			}
			else
			{
				stitchQueue->enqueue(stitch);
				m_usedFlosses[stitch->floss()]++;
			}
		}
		if (stitchQueue->count() == 0)
		{
			delete stitchQueue;
			m_canvasStitches.remove(index);
			return true;
		}
	}
	else
	{
		int stitchCount = stitchQueue->count();
		while (stitchCount--)
		{
			Stitch *stitch = stitchQueue->dequeue();
			m_usedFlosses[stitch->floss()]--;
			if ((stitch->type() == maskStitch) && (!maskColor || (stitch->floss() == m_properties["currentFlossIndex"].toInt())))
			{
				// delete any stitches of the required stitch if it is the correct color or if the color doesn't matter
				delete stitch;
			}
			else
			{
				if ((stitch->type() & maskStitch) && (!maskColor || (stitch->floss() == m_properties["currentFlossIndex"].toInt())) && ((stitch->type() & 192) == 0))
				{
					// the mask covers a part of the current stitch and is the correct color or if the color doesn't matter
					Stitch::Type changeMask = (Stitch::Type)(stitch->type() ^ maskStitch);
					int flossIndex = stitch->floss();
					delete stitch;
					switch (changeMask)
					{
						// changeMask contains what is left of the original stitch after deleting the maskStitch
						// it may contain illegal values, so these are checked for
						case 3:
							stitchQueue->enqueue(new Stitch(Stitch::TLQtr, flossIndex));
							stitchQueue->enqueue(new Stitch(Stitch::TRQtr, flossIndex));
							m_usedFlosses[flossIndex] += 2;
							break;
						case 5:
							stitchQueue->enqueue(new Stitch(Stitch::TLQtr, flossIndex));
							stitchQueue->enqueue(new Stitch(Stitch::BLQtr, flossIndex));
							m_usedFlosses[flossIndex] += 2;
							break;
						case 10:
							stitchQueue->enqueue(new Stitch(Stitch::TRQtr, flossIndex));
							stitchQueue->enqueue(new Stitch(Stitch::BRQtr, flossIndex));
							m_usedFlosses[flossIndex] += 2;
							break;
						case 12:
							stitchQueue->enqueue(new Stitch(Stitch::BLQtr, flossIndex));
							stitchQueue->enqueue(new Stitch(Stitch::BRQtr, flossIndex));
							m_usedFlosses[flossIndex] += 2;
							break;
						default:
							stitchQueue->enqueue(new Stitch((Stitch::Type)changeMask, flossIndex));
							m_usedFlosses[flossIndex]++;
							break;
					}
				}
				else
				{
					stitchQueue->enqueue(stitch);
					m_usedFlosses[stitch->floss()]++;
				}
			}
		}
		return true;
	}
	return false;
}


bool Document::addBackstitch(const QPoint &start, const QPoint &end)
{
	if (validateSnap(start) && validateSnap(end))
	{
		m_canvasBackstitches.append(new Backstitch(start, end, m_properties["currentFlossIndex"].toInt()));
		m_usedFlosses[m_properties["currentFlossIndex"].toInt()]++;
		return true;
	}

	return false;
}



bool Document::deleteBackstitch(const QPoint &start, const QPoint &end, int maskColor)
{
	bool deleted = false;

	for (int i = 0 ; i < m_canvasBackstitches.count() ; ++i)
	{
		if ((m_canvasBackstitches.at(i)->start() == start) && (m_canvasBackstitches.at(i)->end() == end))
		{
			kDebug() << !maskColor << (m_canvasBackstitches.at(i)->floss() == m_properties["currentFlossIndex"].toInt());
			if (!maskColor || (m_canvasBackstitches.at(i)->floss() == m_properties["currentFlossIndex"].toInt()))
			{
				m_usedFlosses[m_canvasBackstitches.at(i)->floss()]--;
				m_canvasBackstitches.removeAt(i);
				deleted = true;
				break;
			}
		}
	}

	return deleted;
}


bool Document::addFrenchKnot(const QPoint &snap)
{
	if (validateSnap(snap))
	{
		m_canvasKnots.append(new Knot(snap, m_properties["currentFlossIndex"].toInt()));
		m_usedFlosses[m_properties["currentFlossIndex"].toInt()]++;
		return true;
	}

	return false;
}


bool Document::deleteFrenchKnot(const QPoint &snap, int maskColor)
{
	bool deleted = false;

	for (int i = 0 ; i < m_canvasKnots.count() ; )
	{
		if (m_canvasKnots.at(i)->position() == snap)
		{
			if (!maskColor || (m_canvasKnots.at(i)->floss() == m_properties["currentFlossIndex"].toInt()))
			{
				m_usedFlosses[m_canvasKnots.at(i)->floss()]--;
				m_canvasKnots.removeAt(i);
				deleted = true;
			}
			else
				i++;
		}
	}

	return deleted;
}


void Document::selectFloss(int flossIndex)
{
	m_properties["currentFlossIndex"] = flossIndex;
}


QUndoStack &Document::undoStack()
{
	return m_undoStack;
}


void Document::clearUnusedColors()
{
	QMutableMapIterator<int, DocumentFloss *> flosses(m_palette);
	while (flosses.hasNext())
	{
		flosses.next();
		if (m_usedFlosses[flosses.key()] == 0)
			flosses.remove();
	}
}


QMap<int, DocumentFloss *> &Document::palette()
{
	return m_palette;
}


QListIterator<BackgroundImage *> Document::backgroundImages() const
{
	return QListIterator<BackgroundImage*>(m_backgroundImages);
}


QListIterator<Backstitch *> Document::backstitches() const
{
	return QListIterator<Backstitch *>(m_canvasBackstitches);
}


QListIterator<Knot *> Document::knots() const
{
	return QListIterator<Knot *>(m_canvasKnots);
}


bool Document::paletteManager()
{
	bool added = false;
	PaletteManagerDlg *paletteManagerDlg = new PaletteManagerDlg(m_schemeManager, m_properties["flossSchemeName"].toString(), m_palette, m_usedFlosses);
	if (paletteManagerDlg->exec())
	{
		added = true;
	}

	return added;
}


void Document::addBackgroundImage(const KUrl &url, const QRect &rect)
{
	BackgroundImage *backgroundImage = new BackgroundImage(url, rect);

	m_backgroundImages.append(backgroundImage);
}


void Document::removeBackgroundImage(const QString &url)
{
	for (int i = 0 ; i < m_backgroundImages.count() ; i++)
	{
		if (m_backgroundImages.at(i)->URL() == url)
		{
			m_backgroundImages.removeAt(i);
			break;
		}
	}
}


void Document::fitBackgroundImage(const QString &url, const QRect &selectionArea)
{
	for (int i = 0 ; i < m_backgroundImages.count() ; i++)
	{
		if (m_backgroundImages.at(i)->URL() == url)
		{
			m_backgroundImages[i]->setLocation(selectionArea);
			break;
		}
	}
}


void Document::showBackgroundImage(const QString &url, bool visible)
{
	for (int i = 0 ; i < m_backgroundImages.count() ; i++)
	{
		if (m_backgroundImages.at(i)->URL() == url)
		{
			m_backgroundImages[i]->setVisible(visible);
			break;
		}
	}
}


unsigned int Document::canvasIndex(const QPoint &cell) const
{
	return cell.y()*m_properties["width"].toInt() + cell.x();
}


bool Document::validateCell(const QPoint &cell) const
{
	if (cell.x() < 0)
		return false;
	if (cell.y() < 0)
		return false;
	if (m_properties["width"].toInt() <= cell.x())
		return false;
	if (m_properties["height"].toInt() <= cell.y())
		return false;
	return true;
}


bool Document::validateSnap(const QPoint &snap) const
{
	if (snap.x() < 0)
		return false;
	if (snap.y() < 0)
		return false;
	if (m_properties["width"].toInt()*2+1 <= snap.x())
		return false;
	if (m_properties["height"].toInt()*2+1 <= snap.y())
		return false;
	return true;
}
