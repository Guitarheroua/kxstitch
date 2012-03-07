/********************************************************************************
 *	Copyright (C) 2010 by Stephen Allewell					*
 *	stephen@mirramar.adsl24.co.uk						*
 *										*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or	*
 *	(at your option) any later version.					*
 ********************************************************************************/


#include "PatternElementDlg.h"

#include <KDebug>

#include "Document.h"
#include "Element.h"
#include "SelectArea.h"


PatternElementDlg::PatternElementDlg(QWidget *parent, PatternElement *patternElement, Document *document, const QList<QRect> &patternRects)
	:	KDialog(parent),
		m_patternElement(patternElement),
		m_document(document),
		m_patternRects(patternRects)
{
	setCaption(i18n("Pattern Element Properties"));
	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);

	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);

	ui.RenderStitchesAs->setItemData(0, Configuration::EnumRenderer_RenderStitchesAs::Stitches);
	ui.RenderStitchesAs->setItemData(1, Configuration::EnumRenderer_RenderStitchesAs::BlackWhiteSymbols);
	ui.RenderStitchesAs->setItemData(2, Configuration::EnumRenderer_RenderStitchesAs::ColorSymbols);
	ui.RenderStitchesAs->setItemData(3, Configuration::EnumRenderer_RenderStitchesAs::ColorBlocks);
	ui.RenderStitchesAs->setItemData(4, Configuration::EnumRenderer_RenderStitchesAs::ColorBlocksSymbols);

	ui.RenderBackstitchesAs->setItemData(0, Configuration::EnumRenderer_RenderBackstitchesAs::ColorLines);
	ui.RenderBackstitchesAs->setItemData(1, Configuration::EnumRenderer_RenderBackstitchesAs::BlackWhiteSymbols);

	ui.RenderKnotsAs->setItemData(0, Configuration::EnumRenderer_RenderKnotsAs::ColorBlocks);
	ui.RenderKnotsAs->setItemData(1, Configuration::EnumRenderer_RenderKnotsAs::ColorSymbols);
	ui.RenderKnotsAs->setItemData(2, Configuration::EnumRenderer_RenderKnotsAs::BlackWhiteSymbols);

	ui.FormatScalesAs->setItemData(0, Configuration::EnumEditor_FormatScalesAs::Stitches);
	ui.FormatScalesAs->setItemData(1, Configuration::EnumEditor_FormatScalesAs::Inches);
	ui.FormatScalesAs->setItemData(2, Configuration::EnumEditor_FormatScalesAs::CM);

	m_selectArea = new SelectArea(ui.PreviewFrame, patternElement, document, patternRects);
	ui.PreviewFrame->setWidget(m_selectArea);

	m_selectArea->setPatternRect(patternElement->m_patternRect);
	ui.ShowStitches->setChecked(patternElement->m_showStitches);
	ui.ShowBackstitches->setChecked(patternElement->m_showBackstitches);
	ui.ShowKnots->setChecked(patternElement->m_showKnots);
	ui.ShowGrid->setChecked(patternElement->m_showGrid);
	ui.ShowScales->setChecked(patternElement->m_showScales);
	ui.ShowPlan->setChecked(patternElement->m_showPlan);
	ui.FormatScalesAs->setCurrentIndex(ui.FormatScalesAs->findData(patternElement->m_formatScalesAs));
	ui.RenderStitchesAs->setCurrentIndex(ui.RenderStitchesAs->findData(patternElement->m_renderStitchesAs));
	ui.RenderBackstitchesAs->setCurrentIndex(ui.RenderBackstitchesAs->findData(patternElement->m_renderBackstitchesAs));
	ui.RenderKnotsAs->setCurrentIndex(ui.RenderKnotsAs->findData(patternElement->m_renderKnotsAs));

	QMetaObject::connectSlotsByName(this);

	setMainWidget(widget);
}


PatternElementDlg::~PatternElementDlg()
{
}


bool PatternElementDlg::showPlan() const
{
	return ui.ShowPlan->isChecked();
}


PlanElement *PatternElementDlg::planElement() const
{
	return m_patternElement->m_planElement;
}


void PatternElementDlg::slotButtonClicked(int button)
{
	if (button == KDialog::Ok)
	{
		m_patternElement->setPatternRect(m_selectArea->patternRect());
		m_patternElement->m_showScales = ui.ShowScales->isChecked();
		if (ui.ShowPlan->isChecked() && m_patternElement->m_planElement == 0)
			m_patternElement->m_planElement = new PlanElement(m_patternElement->parent(), QRect(m_patternElement->rectangle().topRight()-QPoint(50, 50), QSize(50, 50)));
		if (m_patternElement->m_planElement)
			m_patternElement->m_planElement->setPatternRect(m_patternElement->patternRect());
		m_patternElement->m_showPlan = ui.ShowPlan->isChecked();
		m_patternElement->m_formatScalesAs = static_cast<Configuration::EnumEditor_FormatScalesAs::type>(ui.FormatScalesAs->itemData(ui.FormatScalesAs->currentIndex()).toInt());
		m_patternElement->setRenderStitchesAs(static_cast<Configuration::EnumRenderer_RenderStitchesAs::type>(ui.RenderStitchesAs->itemData(ui.RenderStitchesAs->currentIndex()).toInt()));
		m_patternElement->setRenderBackstitchesAs(static_cast<Configuration::EnumRenderer_RenderBackstitchesAs::type>(ui.RenderBackstitchesAs->itemData(ui.RenderBackstitchesAs->currentIndex()).toInt()));
		m_patternElement->setRenderKnotsAs(static_cast<Configuration::EnumRenderer_RenderKnotsAs::type>(ui.RenderKnotsAs->itemData(ui.RenderKnotsAs->currentIndex()).toInt()));
		m_patternElement->m_showStitches = ui.ShowStitches->isChecked();
		m_patternElement->m_showBackstitches = ui.ShowBackstitches->isChecked();
		m_patternElement->m_showKnots = ui.ShowKnots->isChecked();
		m_patternElement->m_showGrid = ui.ShowGrid->isChecked();
		accept();
	}
	else
		KDialog::slotButtonClicked(button);
}