/*
    SPDX-FileCopyrightText: 2024 The Okular Authors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BLOCKSELECTION_H
#define BLOCKSELECTION_H

#include <QList>
#include <QMap>

#include "area.h"
#include "okularcore_export.h"
#include "textpage.h"

namespace Okular
{

/**
 * @brief Helper class for block-aware text selection in multi-column layouts.
 *
 * This class provides utility functions to constrain text selection within
 * layout blocks, respecting reading order for multi-column documents.
 *
 * Layout blocks are extracted by external AI services (e.g., Surya) and
 * embedded as XMP metadata in PDFs. Okular reads this metadata and uses
 * it to provide intelligent text selection that stays within columns.
 */
class OKULARCORE_EXPORT BlockSelectionHelper
{
public:
    /**
     * Find the block containing a specific point.
     *
     * @param blocks List of layout blocks to search
     * @param p Point to find (in normalized coordinates)
     * @return Pointer to the containing block, or nullptr if none
     */
    static const LayoutBlock *findBlockContaining(const QList<LayoutBlock> &blocks, const NormalizedPoint &p);

    /**
     * Find the block that should be considered "active" for a cursor position.
     *
     * If the point is inside a block, returns that block. Otherwise, finds
     * the block that would be appropriate based on reading order - essentially
     * the block whose "reading territory" the cursor is in.
     *
     * @param blocks List of layout blocks
     * @param p The cursor position in normalized coordinates
     * @return Pointer to the appropriate block, or nullptr if none found
     */
    static const LayoutBlock *findBlockForCursor(const QList<LayoutBlock> &blocks, const NormalizedPoint &p);

    /**
     * Get the next block in reading order.
     *
     * @param blocks List of layout blocks
     * @param current The current block
     * @return Pointer to the next block, or nullptr if none
     */
    static const LayoutBlock *getNextBlock(const QList<LayoutBlock> &blocks, const LayoutBlock *current);

    /**
     * Collect all blocks within a reading order range.
     *
     * @param blocks List of layout blocks
     * @param minOrder Minimum reading order (inclusive)
     * @param maxOrder Maximum reading order (inclusive)
     * @return List of pointers to blocks in the range
     */
    static QList<const LayoutBlock *> getBlocksInReadingOrderRange(const QList<LayoutBlock> &blocks, int minOrder, int maxOrder);

    /**
     * Check if an entity's center falls within any of the given blocks.
     *
     * @param entity The text entity to check
     * @param blocks List of blocks to check against
     * @return true if the entity's center is in any of the blocks
     */
    static bool isEntityInAnyBlock(const NormalizedRect &entityArea, const QList<const LayoutBlock *> &blocks);

    /**
     * Extract text from entities, respecting block reading order.
     *
     * Entities are grouped by block, sorted within each block by geometric
     * position (Y then X), and concatenated in reading order.
     *
     * @param words The list of text entities
     * @param blocks The layout blocks defining reading order
     * @param area The selection area (entities must intersect/contain this)
     * @param useIntersects If true, use intersection test; if false, use center containment
     * @return Text extracted in reading order
     */
    static QString extractTextInReadingOrder(const TextEntity::List &words, const QList<LayoutBlock> &blocks, const RegularAreaRect *area, bool useIntersects);

    /**
     * Get block IDs for a text selection.
     *
     * Uses reading order range between start and end blocks, matching
     * the algorithm used by TextPage::textArea() for text selection.
     *
     * @param blocks List of layout blocks
     * @param selectionStart The point where user started the selection (normalized)
     * @param selectionEnd The point where user ended the selection (normalized)
     * @return List of block IDs in reading order (empty if no blocks found)
     */
    static QStringList getBlockIdsForSelection(const QList<LayoutBlock> &blocks, const NormalizedPoint &selectionStart, const NormalizedPoint &selectionEnd);
};

} // namespace Okular

#endif // BLOCKSELECTION_H
