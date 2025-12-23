/*
    SPDX-FileCopyrightText: 2024 Okular Contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../core/area.h"
#include "../core/page.h"
#include "../core/textpage.h"

class TextPageBlockTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLayoutBlockDefaultConstructor();
    void testLayoutBlockParameterizedConstructor();
    void testLayoutBlockContainsPoint();
    void testLayoutBlockContainsPointEdgeCases();
    void testLayoutBlockContainsRect();
    void testLayoutBlockContainsRectSpanningBoundary();
    void testTextPageWithoutBlocks();
    void testTextPageWithBlocks();
    void testTextPageSetLayoutBlocks();
    void testBlockReadingOrder();
    void testMultipleBlocksOnPage();
};

void TextPageBlockTest::testLayoutBlockDefaultConstructor()
{
    Okular::LayoutBlock block;

    // Default values should be set
    QVERIFY(block.id.isEmpty());
    QCOMPARE(block.page, -1);
    QCOMPARE(block.readingOrder, -1);
    QCOMPARE(block.confidence, 0.0);
    QVERIFY(block.blockType.isEmpty());
}

void TextPageBlockTest::testLayoutBlockParameterizedConstructor()
{
    Okular::NormalizedRect bbox(0.1, 0.2, 0.5, 0.8);
    Okular::LayoutBlock block(
        QStringLiteral("test_block_1"),
        0,
        bbox,
        QStringLiteral("TEXT"),
        0,
        0.95
    );

    QCOMPARE(block.id, QStringLiteral("test_block_1"));
    QCOMPARE(block.page, 0);
    QCOMPARE(block.blockType, QStringLiteral("TEXT"));
    QCOMPARE(block.readingOrder, 0);
    QCOMPARE(block.confidence, 0.95);

    // Verify bbox coordinates
    QCOMPARE(block.bbox.left, 0.1);
    QCOMPARE(block.bbox.top, 0.2);
    QCOMPARE(block.bbox.right, 0.5);
    QCOMPARE(block.bbox.bottom, 0.8);
}

void TextPageBlockTest::testLayoutBlockContainsPoint()
{
    // Create a block covering left half of page (x: 0.0-0.45)
    Okular::LayoutBlock block;
    block.id = QStringLiteral("test_block");
    block.page = 0;
    block.bbox = Okular::NormalizedRect(0.0, 0.0, 0.45, 1.0);
    block.blockType = QStringLiteral("TEXT");
    block.readingOrder = 0;
    block.confidence = 1.0;

    // Point clearly inside block
    Okular::NormalizedPoint insidePoint(0.2, 0.5);
    QVERIFY(block.contains(insidePoint));

    // Point clearly outside block (in right half)
    Okular::NormalizedPoint outsidePoint(0.7, 0.5);
    QVERIFY(!block.contains(outsidePoint));

    // Point at top-left corner (should be inside)
    Okular::NormalizedPoint cornerPoint(0.0, 0.0);
    QVERIFY(block.contains(cornerPoint));

    // Point at bottom-right edge (should be inside)
    Okular::NormalizedPoint edgePoint(0.45, 1.0);
    QVERIFY(block.contains(edgePoint));
}

void TextPageBlockTest::testLayoutBlockContainsPointEdgeCases()
{
    // Create a small block
    Okular::LayoutBlock block;
    block.bbox = Okular::NormalizedRect(0.25, 0.25, 0.75, 0.75);

    // Test points just inside boundaries
    QVERIFY(block.contains(Okular::NormalizedPoint(0.25, 0.5)));  // left edge
    QVERIFY(block.contains(Okular::NormalizedPoint(0.75, 0.5)));  // right edge
    QVERIFY(block.contains(Okular::NormalizedPoint(0.5, 0.25)));  // top edge
    QVERIFY(block.contains(Okular::NormalizedPoint(0.5, 0.75)));  // bottom edge

    // Test point just outside left boundary
    QVERIFY(!block.contains(Okular::NormalizedPoint(0.24, 0.5)));

    // Test point just outside right boundary
    QVERIFY(!block.contains(Okular::NormalizedPoint(0.76, 0.5)));
}

void TextPageBlockTest::testLayoutBlockContainsRect()
{
    Okular::LayoutBlock block;
    block.bbox = Okular::NormalizedRect(0.0, 0.0, 0.5, 1.0);

    // Rect with center clearly inside block
    Okular::NormalizedRect insideRect(0.1, 0.3, 0.3, 0.7);
    // Center is at (0.2, 0.5) - inside block
    QVERIFY(block.contains(insideRect));

    // Rect with center clearly outside block
    Okular::NormalizedRect outsideRect(0.6, 0.3, 0.9, 0.7);
    // Center is at (0.75, 0.5) - outside block
    QVERIFY(!block.contains(outsideRect));
}

void TextPageBlockTest::testLayoutBlockContainsRectSpanningBoundary()
{
    Okular::LayoutBlock block;
    block.bbox = Okular::NormalizedRect(0.0, 0.0, 0.5, 1.0);

    // Rect spanning block boundary, center inside
    Okular::NormalizedRect spanningInsideRect(0.3, 0.3, 0.6, 0.7);
    // Center is at (0.45, 0.5) - inside block
    QVERIFY(block.contains(spanningInsideRect));

    // Rect spanning block boundary, center outside
    Okular::NormalizedRect spanningOutsideRect(0.4, 0.3, 0.8, 0.7);
    // Center is at (0.6, 0.5) - outside block
    QVERIFY(!block.contains(spanningOutsideRect));

    // Rect at exact boundary - center exactly at edge
    Okular::NormalizedRect boundaryRect(0.4, 0.3, 0.6, 0.7);
    // Center is at (0.5, 0.5) - exactly at right edge of block
    QVERIFY(block.contains(boundaryRect));
}

void TextPageBlockTest::testTextPageWithoutBlocks()
{
    // Create TextPage without layout blocks
    Okular::TextPage textPage;

    // Add some text entities
    textPage.append(QStringLiteral("Hello"), Okular::NormalizedRect(0.1, 0.1, 0.2, 0.15));
    textPage.append(QStringLiteral("World"), Okular::NormalizedRect(0.3, 0.1, 0.4, 0.15));

    // Should not have layout blocks
    QVERIFY(!textPage.hasLayoutBlocks());
}

void TextPageBlockTest::testTextPageWithBlocks()
{
    Okular::TextPage textPage;

    // Add layout blocks for two-column layout
    QList<Okular::LayoutBlock> blocks;
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left"),
        0,
        Okular::NormalizedRect(0.0, 0.0, 0.45, 1.0),
        QStringLiteral("TEXT"),
        0,
        1.0
    ));
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right"),
        0,
        Okular::NormalizedRect(0.55, 0.0, 1.0, 1.0),
        QStringLiteral("TEXT"),
        1,
        1.0
    ));

    textPage.setLayoutBlocks(blocks);

    QVERIFY(textPage.hasLayoutBlocks());
}

void TextPageBlockTest::testTextPageSetLayoutBlocks()
{
    Okular::TextPage textPage;

    // Initially no blocks
    QVERIFY(!textPage.hasLayoutBlocks());

    // Add blocks
    QList<Okular::LayoutBlock> blocks;
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("block1"),
        0,
        Okular::NormalizedRect(0.0, 0.0, 1.0, 0.5),
        QStringLiteral("TEXT"),
        0,
        0.9
    ));

    textPage.setLayoutBlocks(blocks);
    QVERIFY(textPage.hasLayoutBlocks());

    // Clear blocks by setting empty list
    textPage.setLayoutBlocks(QList<Okular::LayoutBlock>());
    QVERIFY(!textPage.hasLayoutBlocks());
}

void TextPageBlockTest::testBlockReadingOrder()
{
    // Test that blocks maintain reading order
    QList<Okular::LayoutBlock> blocks;

    // Add blocks in non-sequential order to verify readingOrder is independent of list position
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("third"),
        0,
        Okular::NormalizedRect(0.0, 0.5, 0.5, 1.0),
        QStringLiteral("TEXT"),
        2,  // Reading order 2
        1.0
    ));
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("first"),
        0,
        Okular::NormalizedRect(0.0, 0.0, 0.5, 0.5),
        QStringLiteral("TEXT"),
        0,  // Reading order 0
        1.0
    ));
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("second"),
        0,
        Okular::NormalizedRect(0.5, 0.0, 1.0, 0.5),
        QStringLiteral("TEXT"),
        1,  // Reading order 1
        1.0
    ));

    // Verify reading order is stored correctly
    QCOMPARE(blocks[0].readingOrder, 2);
    QCOMPARE(blocks[0].id, QStringLiteral("third"));

    QCOMPARE(blocks[1].readingOrder, 0);
    QCOMPARE(blocks[1].id, QStringLiteral("first"));

    QCOMPARE(blocks[2].readingOrder, 1);
    QCOMPARE(blocks[2].id, QStringLiteral("second"));
}

void TextPageBlockTest::testMultipleBlocksOnPage()
{
    // Test a typical two-column layout
    Okular::TextPage textPage;

    QList<Okular::LayoutBlock> blocks;

    // Left column
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left_col"),
        0,
        Okular::NormalizedRect(0.05, 0.1, 0.45, 0.9),
        QStringLiteral("TEXT"),
        0,
        0.98
    ));

    // Right column
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right_col"),
        0,
        Okular::NormalizedRect(0.55, 0.1, 0.95, 0.9),
        QStringLiteral("TEXT"),
        1,
        0.97
    ));

    // Header (spans both columns)
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("header"),
        0,
        Okular::NormalizedRect(0.05, 0.02, 0.95, 0.08),
        QStringLiteral("TEXT"),
        -1,  // Before main content
        0.99
    ));

    textPage.setLayoutBlocks(blocks);
    QVERIFY(textPage.hasLayoutBlocks());

    // Test point containment for left column
    Okular::NormalizedPoint leftPoint(0.25, 0.5);
    QVERIFY(blocks[0].contains(leftPoint));
    QVERIFY(!blocks[1].contains(leftPoint));

    // Test point containment for right column
    Okular::NormalizedPoint rightPoint(0.75, 0.5);
    QVERIFY(!blocks[0].contains(rightPoint));
    QVERIFY(blocks[1].contains(rightPoint));

    // Test point in header
    Okular::NormalizedPoint headerPoint(0.5, 0.05);
    QVERIFY(!blocks[0].contains(headerPoint));
    QVERIFY(!blocks[1].contains(headerPoint));
    QVERIFY(blocks[2].contains(headerPoint));

    // Test point in gap between columns (should not be in any column block)
    Okular::NormalizedPoint gapPoint(0.5, 0.5);
    QVERIFY(!blocks[0].contains(gapPoint));
    QVERIFY(!blocks[1].contains(gapPoint));
}

QTEST_MAIN(TextPageBlockTest)
#include "textpageblocktest.moc"
