/********************************************************************************
 *	Copyright (C) 2010 by Stephen Allewell					*
 *	stephen@mirramar.fsnet.co.uk						*
 *										*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or	*
 *	(at your option) any later version.					*
 ********************************************************************************/


#ifndef __FlossScheme_h
#define __FlossScheme_H


#include <QColor>
#include <QList>
#include <QListIterator>
#include <QString>

#include <Magick++.h>

#include "Floss.h"


class FlossScheme
{
	public:
		FlossScheme();
		~FlossScheme();

		Floss *convert(const QColor &color);
		Floss *find(const QString &name) const;
		QString find(const QColor &color) const;
		QString schemeName() const;
		const QList<Floss *> &flosses() const;

		void addFloss(Floss *floss);
		void clearScheme();
		Magick::Image *createImageMap();
		void setSchemeName(const QString &name);

	private:
		QString		m_schemeName;
		QList<Floss *>	m_flosses;
		Magick::Image	*m_map;
};

#endif
