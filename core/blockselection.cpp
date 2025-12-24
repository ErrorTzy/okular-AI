/*
    SPDX-FileCopyrightText: 2024 The Okular Authors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "blockselection.h"

#include <climits>

#include <QtAlgorithms>

using namespace Okular;

const LayoutBlock *BlockSelectionHelper::findBlockContaining(const QList<LayoutBlock> &blocks, const NormalizedPoint &p)
{
    for (const auto &block : blocks) {
        if (block.contains(p)) {
            return &block;
        }
    }
    return nullptr;
}

const LayoutBlock *BlockSelectionHelper::getNextBlock(const QList<LayoutBlock> &blocks, const LayoutBlock *current)
{
    if (!current) {
        return nullptr;
    }

    for (const auto &block : blocks) {
        if (block.readingOrder == current->readingOrder + 1) {
            return &block;
        }
    }
    return nullptr;
}

const LayoutBlock *BlockSelectionHelper::findBlockForCursor(const QList<LayoutBlock> &blocks, const NormalizedPoint &p)
{
    // First, check if cursor is directly inside a block
    const LayoutBlock *directBlock = findBlockContaining(blocks, p);
    if (directBlock) {
        return directBlock;
    }

    // Cursor is not inside any block - find the most appropriate block
    // based on reading order and position.
    //
    // Strategy: Find the block with the highest reading order that the cursor
    // has "passed" - meaning the cursor is below the block's bottom edge,
    // or the cursor is to the right of the block and on the same vertical level.

    const LayoutBlock *bestBlock = nullptr;
    int bestOrder = -1;

    for (const auto &block : blocks) {
        // Check if cursor has "passed" this block in reading order terms
        bool passed = false;

        // Case 1: Cursor is below the block's bottom edge
        if (p.y > block.bbox.bottom) {
            passed = true;
        }
        // Case 2: Cursor is at the same vertical level but to the right
        else if (p.y >= block.bbox.top && p.y <= block.bbox.bottom && p.x > block.bbox.right) {
            passed = true;
        }

        if (passed && block.readingOrder > bestOrder) {
            bestOrder = block.readingOrder;
            bestBlock = &block;
        }
    }

    // If we found a block we've passed, return the NEXT block in reading order
    // (since that's where selection should continue to)
    if (bestBlock) {
        const LayoutBlock *nextBlock = getNextBlock(blocks, bestBlock);
        if (nextBlock) {
            return nextBlock;
        }
        // If there's no next block, stay with the best block we found
        return bestBlock;
    }

    // No block passed - return the first block (lowest reading order)
    if (!blocks.isEmpty()) {
        const LayoutBlock *firstBlock = nullptr;
        int minOrder = INT_MAX;
        for (const auto &block : blocks) {
            if (block.readingOrder >= 0 && block.readingOrder < minOrder) {
                minOrder = block.readingOrder;
                firstBlock = &block;
            }
        }
        return firstBlock;
    }

    return nullptr;
}

QList<const LayoutBlock *> BlockSelectionHelper::getBlocksInReadingOrderRange(const QList<LayoutBlock> &blocks, int minOrder, int maxOrder)
{
    QList<const LayoutBlock *> result;

    for (const auto &block : blocks) {
        if (block.readingOrder >= minOrder && block.readingOrder <= maxOrder) {
            result.append(&block);
        }
    }

    return result;
}

bool BlockSelectionHelper::isEntityInAnyBlock(const NormalizedRect &entityArea, const QList<const LayoutBlock *> &blocks)
{
    if (blocks.isEmpty()) {
        return true; // No blocks to check against means include everything
    }

    double cx = (entityArea.left + entityArea.right) / 2.0;
    double cy = (entityArea.top + entityArea.bottom) / 2.0;

    for (const LayoutBlock *block : blocks) {
        if (block && block->bbox.contains(cx, cy)) {
            return true;
        }
    }

    return false;
}

QString BlockSelectionHelper::extractTextInReadingOrder(const TextEntity::List &words, const QList<LayoutBlock> &blocks, const RegularAreaRect *area, bool useIntersects)
{
    // Collect matching entities grouped by block
    QMap<int, QList<const TextEntity *>> entitiesByBlock; // reading order -> entities

    for (auto it = words.constBegin(); it != words.constEnd(); ++it) {
        bool matches = false;
        if (useIntersects) {
            matches = area->intersects(it->area());
        } else {
            NormalizedPoint center = it->area().center();
            matches = area->contains(center.x, center.y);
        }

        if (matches) {
            // Find which block this entity belongs to
            int blockOrder = -1;
            for (const LayoutBlock &block : blocks) {
                if (block.contains(it->area())) {
                    blockOrder = block.readingOrder;
                    break;
                }
            }

            if (blockOrder >= 0) {
                entitiesByBlock[blockOrder].append(&(*it));
            } else {
                // Entity not in any block - use a very high order
                entitiesByBlock[999999].append(&(*it));
            }
        }
    }

    // Sort entities within each block by geometric position (Y, then X)
    for (auto &entities : entitiesByBlock) {
        std::sort(entities.begin(), entities.end(), [](const TextEntity *a, const TextEntity *b) {
            double aY = (a->area().top + a->area().bottom) / 2.0;
            double bY = (b->area().top + b->area().bottom) / 2.0;
            if (qAbs(aY - bY) > 0.01) {
                return aY < bY; // Sort by Y first
            }
            return a->area().left < b->area().left; // Then by X
        });
    }

    // Concatenate text in reading order
    QString result;
    for (auto it = entitiesByBlock.constBegin(); it != entitiesByBlock.constEnd(); ++it) {
        for (const TextEntity *entity : it.value()) {
            result += entity->text();
        }
    }

    return result;
}
