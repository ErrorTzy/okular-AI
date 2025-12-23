/*
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2007 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _OKULAR_TEXTPAGE_P_H_
#define _OKULAR_TEXTPAGE_P_H_

#include "textpage.h"

#include <QList>
#include <QMap>
#include <QPair>
#include <QTransform>

#include "area.h"

class SearchPoint;

class RegionText;

namespace Okular
{
class PagePrivate;
class RegularAreaRect;
class Page;

/**
 * Returns whether the two strings match.
 * Satisfies the condition that if two strings match then their lengths are equal.
 */
typedef bool (*TextComparisonFunction)(QStringView from, const QStringView to);

/**
 * A list of RegionText. It keeps a bunch of TextList with their bounding rectangles
 */
typedef QList<RegionText> RegionTextList;

class TextPagePrivate
{
public:
    TextPagePrivate();
    ~TextPagePrivate();

    RegularAreaRect *findTextInternalForward(int searchID, const QString &query, TextComparisonFunction comparer, const TextEntity::List::ConstIterator start, int start_offset, const TextEntity::List::ConstIterator end);
    RegularAreaRect *findTextInternalBackward(int searchID, const QString &query, TextComparisonFunction comparer, const TextEntity::List::ConstIterator start, int start_offset, const TextEntity::List::ConstIterator end);

    /**
     * Copy a TextList to m_words, the pointers of list are adopted
     */
    void setWordList(const TextEntity::List &list);

    /**
     * Make necessary modifications in the TextList to make the text order correct, so
     * that textselection works fine
     */
    void correctTextOrder();

    /**
     * Find the layout block containing the given normalized point.
     *
     * @param p The point to search for in normalized coordinates
     * @return Pointer to the containing LayoutBlock, or nullptr if not found
     */
    const LayoutBlock *findBlockContaining(const NormalizedPoint &p) const;

    /**
     * Find the layout block containing the center of the given rectangle.
     *
     * @param r The rectangle to search for in normalized coordinates
     * @return Pointer to the containing LayoutBlock, or nullptr if not found
     */
    const LayoutBlock *findBlockContaining(const NormalizedRect &r) const;

    /**
     * Get the next layout block in reading order.
     *
     * @param current The current block
     * @return Pointer to the next LayoutBlock, or nullptr if current is last
     */
    const LayoutBlock *getNextBlock(const LayoutBlock *current) const;

    /**
     * Get the previous layout block in reading order.
     *
     * @param current The current block
     * @return Pointer to the previous LayoutBlock, or nullptr if current is first
     */
    const LayoutBlock *getPreviousBlock(const LayoutBlock *current) const;

    /**
     * Check if the given text entity is the last one in the specified block.
     *
     * @param it Iterator to the current text entity
     * @param block The layout block to check against
     * @return true if this is the last entity in the block
     */
    bool isLastEntityInBlock(TextEntity::List::ConstIterator it, const LayoutBlock *block) const;

    /**
     * Determine if a text entity should be included in selection based on
     * whether its center point falls within the given block.
     *
     * This provides better handling for entities that span block boundaries
     * by using the center point rather than requiring the entire entity
     * to be within the block.
     *
     * @param entity The text entity to check
     * @param block The layout block to check against
     * @return true if the entity should be included (center is in block or no block specified)
     */
    bool shouldIncludeEntity(const TextEntity &entity, const LayoutBlock *block) const;

    // variables those can be accessed directly from TextPage
    TextEntity::List m_words;
    QMap<int, SearchPoint *> m_searchPoints;
    Page *m_page;

    /**
     * Layout blocks for constraining text selection.
     * When non-empty, text selection will be constrained to stay within
     * block boundaries, enabling proper selection in multi-column layouts.
     * @since 24.12
     */
    QList<LayoutBlock> m_layoutBlocks;

private:
    RegularAreaRect *searchPointToArea(const SearchPoint *sp);
};

}

#endif
