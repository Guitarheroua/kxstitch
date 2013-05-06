/*
 * Copyright (C) 2010 by Stephen Allewell
 * stephen@mirramar.adsl24.co.uk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef TextElementDlg_H
#define TextElementDlg_H


#include "ui_TextElement.h"


class TextElement;


class TextElementDlg : public KDialog
{
    Q_OBJECT

public:
    TextElementDlg(QWidget *, TextElement *);
    ~TextElementDlg();

protected slots:
    void slotButtonClicked(int);

private slots:
    void on_FillBackground_toggled(bool);
    void on_BackgroundColor_activated(const QColor &);
    void on_BackgroundTransparency_valueChanged(int);

    void on_BoldButton_clicked();
    void on_UnderlineButton_clicked();
    void on_ItalicButton_clicked();
    void on_FontFamily_currentFontChanged(const QFont &);
    void on_PointSize_valueChanged(int);
    void on_TextColor_clicked();
    void textAlign(QAbstractButton *);

    void on_Text_currentCharFormatChanged(const QTextCharFormat &);
    void on_Text_cursorPositionChanged();

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &);
    void fontChanged(const QFont &);
    void colorChanged(const QColor &);
    void alignmentChanged(Qt::Alignment);

    Ui::TextElement ui;

    TextElement *m_textElement;
};


#endif // TextElementDlg_H
