/*
    SPDX-FileCopyrightText: 2024 Okular Contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../core/area.h"
#include "../core/misc.h"
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
    // Cross-block selection tests
    void testCrossBlockSelectionTwoColumn();
    void testCrossBlockSelectionWithFullWidthBlocks();
    void testCrossBlockSelectionReadingOrderJump();
    void testCrossBlockSelectionLinePrecision();
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

void TextPageBlockTest::testCrossBlockSelectionTwoColumn()
{
    // Test cross-block selection in a two-column layout
    // Layout:
    //   [Block 0: left column]  [Block 1: right column]
    //
    // Selection from left column to right column should include both blocks

    // Create a page with dimensions
    Okular::Page page(0, 1000, 1000, Okular::Rotation0);

    // Create text entities in both columns
    Okular::TextEntity::List words;
    // Left column words (block 0)
    words.append(Okular::TextEntity(QStringLiteral("Left"), Okular::NormalizedRect(0.1, 0.1, 0.2, 0.15)));
    words.append(Okular::TextEntity(QStringLiteral("Column"), Okular::NormalizedRect(0.1, 0.2, 0.25, 0.25)));
    words.append(Okular::TextEntity(QStringLiteral("Text"), Okular::NormalizedRect(0.1, 0.3, 0.2, 0.35)));
    // Right column words (block 1)
    words.append(Okular::TextEntity(QStringLiteral("Right"), Okular::NormalizedRect(0.6, 0.1, 0.7, 0.15)));
    words.append(Okular::TextEntity(QStringLiteral("Column"), Okular::NormalizedRect(0.6, 0.2, 0.75, 0.25)));
    words.append(Okular::TextEntity(QStringLiteral("Content"), Okular::NormalizedRect(0.6, 0.3, 0.75, 0.35)));

    Okular::TextPage *textPage = new Okular::TextPage(words);

    // Set up two-column layout blocks
    QList<Okular::LayoutBlock> blocks;
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left"),
        0,
        Okular::NormalizedRect(0.0, 0.0, 0.45, 1.0),
        QStringLiteral("TEXT"),
        0,  // Reading order 0
        1.0));
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right"),
        0,
        Okular::NormalizedRect(0.55, 0.0, 1.0, 1.0),
        QStringLiteral("TEXT"),
        1,  // Reading order 1
        1.0));

    textPage->setLayoutBlocks(blocks);
    page.setTextPage(textPage);

    // Test 1: Selection within left column only
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.1, 0.1),
            Okular::NormalizedPoint(0.25, 0.25));
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);
        // Should only include left column entities
        QString text = textPage->text(area.get());
        QVERIFY(text.contains(QStringLiteral("Left")));
        QVERIFY(text.contains(QStringLiteral("Column")));
        QVERIFY(!text.contains(QStringLiteral("Right")));
    }

    // Test 2: Selection spanning both columns
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.1, 0.1),
            Okular::NormalizedPoint(0.75, 0.35));
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);
        // Should include both columns' entities
        QString text = textPage->text(area.get());
        QVERIFY(text.contains(QStringLiteral("Left")));
        QVERIFY(text.contains(QStringLiteral("Right")));
    }
}

void TextPageBlockTest::testCrossBlockSelectionWithFullWidthBlocks()
{
    // Test the scenario from CLAUDE.md:
    // BLOCK1 (full width, reading order 0)
    // BLOCK2   BLOCK4  (left col: order 1, right col: order 3)
    // BLOCK3   BLOCK5  (left col: order 2, right col: order 4)
    // BLOCK6 (full width, reading order 5)

    Okular::Page page(0, 1000, 1000, Okular::Rotation0);

    // Create text entities
    Okular::TextEntity::List words;
    // Block 1 (full width header)
    words.append(Okular::TextEntity(QStringLiteral("Header"), Okular::NormalizedRect(0.1, 0.02, 0.9, 0.08)));
    // Block 2 (left column, top)
    words.append(Okular::TextEntity(QStringLiteral("LeftTop"), Okular::NormalizedRect(0.1, 0.15, 0.35, 0.25)));
    // Block 3 (left column, bottom)
    words.append(Okular::TextEntity(QStringLiteral("LeftBot"), Okular::NormalizedRect(0.1, 0.35, 0.35, 0.45)));
    // Block 4 (right column, top)
    words.append(Okular::TextEntity(QStringLiteral("RightTop"), Okular::NormalizedRect(0.6, 0.15, 0.85, 0.25)));
    // Block 5 (right column, bottom)
    words.append(Okular::TextEntity(QStringLiteral("RightBot"), Okular::NormalizedRect(0.6, 0.35, 0.85, 0.45)));
    // Block 6 (full width footer)
    words.append(Okular::TextEntity(QStringLiteral("Footer"), Okular::NormalizedRect(0.1, 0.55, 0.9, 0.65)));

    Okular::TextPage *textPage = new Okular::TextPage(words);

    // Set up blocks with proper reading order
    QList<Okular::LayoutBlock> blocks;
    // Block 1: Header (full width)
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("header"), 0,
        Okular::NormalizedRect(0.0, 0.0, 1.0, 0.1),
        QStringLiteral("TEXT"), 0, 1.0));
    // Block 2: Left column top
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left_top"), 0,
        Okular::NormalizedRect(0.0, 0.1, 0.45, 0.3),
        QStringLiteral("TEXT"), 1, 1.0));
    // Block 3: Left column bottom
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left_bot"), 0,
        Okular::NormalizedRect(0.0, 0.3, 0.45, 0.5),
        QStringLiteral("TEXT"), 2, 1.0));
    // Block 4: Right column top
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right_top"), 0,
        Okular::NormalizedRect(0.55, 0.1, 1.0, 0.3),
        QStringLiteral("TEXT"), 3, 1.0));
    // Block 5: Right column bottom
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right_bot"), 0,
        Okular::NormalizedRect(0.55, 0.3, 1.0, 0.5),
        QStringLiteral("TEXT"), 4, 1.0));
    // Block 6: Footer (full width)
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("footer"), 0,
        Okular::NormalizedRect(0.0, 0.5, 1.0, 0.7),
        QStringLiteral("TEXT"), 5, 1.0));

    textPage->setLayoutBlocks(blocks);
    page.setTextPage(textPage);

    // Test: Selection from Block 3 (left bottom) to Block 6 (footer)
    // Should include all blocks in reading order range (2-5): Block 3, Block 4, Block 5, Block 6
    // When crossing block boundaries, ALL entities from blocks in the range are included
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.1, 0.35),   // Start in Block 3 (reading order 2)
            Okular::NormalizedPoint(0.9, 0.65));  // End in Block 6 (reading order 5)
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);

        QString text = textPage->text(area.get());
        // Should include all blocks from reading order 2 to 5
        QVERIFY(text.contains(QStringLiteral("LeftBot")));   // Block 3 (order 2)
        QVERIFY(text.contains(QStringLiteral("RightTop")));  // Block 4 (order 3)
        QVERIFY(text.contains(QStringLiteral("RightBot")));  // Block 5 (order 4)
        QVERIFY(text.contains(QStringLiteral("Footer")));    // Block 6 (order 5)
        // Should NOT include header (Block 1, order 0) or LeftTop (Block 2, order 1)
        QVERIFY(!text.contains(QStringLiteral("Header")));
        QVERIFY(!text.contains(QStringLiteral("LeftTop")));
    }
}

void TextPageBlockTest::testCrossBlockSelectionReadingOrderJump()
{
    // Test that selection from bottom of left column to footer
    // correctly includes right column blocks
    //
    // This tests the "column jump" scenario where:
    // - User selects from Block 3 (bottom of left column)
    // - Drags to position in Block 6's area
    // - Selection should include Blocks 4 and 5 (right column)

    Okular::Page page(0, 1000, 1000, Okular::Rotation0);

    Okular::TextEntity::List words;
    // Left column
    words.append(Okular::TextEntity(QStringLiteral("L1"), Okular::NormalizedRect(0.1, 0.1, 0.2, 0.2)));
    words.append(Okular::TextEntity(QStringLiteral("L2"), Okular::NormalizedRect(0.1, 0.25, 0.2, 0.35)));
    words.append(Okular::TextEntity(QStringLiteral("L3"), Okular::NormalizedRect(0.1, 0.4, 0.2, 0.5)));
    // Right column
    words.append(Okular::TextEntity(QStringLiteral("R1"), Okular::NormalizedRect(0.6, 0.1, 0.7, 0.2)));
    words.append(Okular::TextEntity(QStringLiteral("R2"), Okular::NormalizedRect(0.6, 0.25, 0.7, 0.35)));
    words.append(Okular::TextEntity(QStringLiteral("R3"), Okular::NormalizedRect(0.6, 0.4, 0.7, 0.5)));
    // Footer
    words.append(Okular::TextEntity(QStringLiteral("Footer"), Okular::NormalizedRect(0.1, 0.6, 0.9, 0.7)));

    Okular::TextPage *textPage = new Okular::TextPage(words);

    QList<Okular::LayoutBlock> blocks;
    // Left column
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left"), 0,
        Okular::NormalizedRect(0.0, 0.0, 0.45, 0.55),
        QStringLiteral("TEXT"), 0, 1.0));
    // Right column
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right"), 0,
        Okular::NormalizedRect(0.55, 0.0, 1.0, 0.55),
        QStringLiteral("TEXT"), 1, 1.0));
    // Footer
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("footer"), 0,
        Okular::NormalizedRect(0.0, 0.55, 1.0, 0.75),
        QStringLiteral("TEXT"), 2, 1.0));

    textPage->setLayoutBlocks(blocks);
    page.setTextPage(textPage);

    // Test: Select from L3 (bottom of left column) to Footer
    // Cross-block selection behavior:
    // - Start block: partial selection from cursor position onwards
    // - Intermediate blocks: all entities included
    // - End block: partial selection up to cursor position
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.1, 0.4),   // Start at L3
            Okular::NormalizedPoint(0.9, 0.7));  // End at Footer
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);

        QString text = textPage->text(area.get());
        // Start block: only L3 (from cursor position onwards), NOT L1 or L2
        QVERIFY(!text.contains(QStringLiteral("L1")));  // Before cursor
        QVERIFY(!text.contains(QStringLiteral("L2")));  // Before cursor
        QVERIFY(text.contains(QStringLiteral("L3")));   // At cursor
        // Intermediate block: entire right column (reading order 1)
        QVERIFY(text.contains(QStringLiteral("R1")));
        QVERIFY(text.contains(QStringLiteral("R2")));
        QVERIFY(text.contains(QStringLiteral("R3")));
        // End block: footer (at cursor position)
        QVERIFY(text.contains(QStringLiteral("Footer")));
    }

    // Test: Select from L1 to L3 (within same block)
    // Since we're using a fresh selection, this should only include left column
    // NOTE: Due to how textArea() works with the geometric start/end iterators,
    // when minOrder == maxOrder (single block), we iterate from geometric start
    // to geometric end and filter by block membership.
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.1, 0.1),
            Okular::NormalizedPoint(0.35, 0.5));  // Extended to stay clearly in left column
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);

        QString text = textPage->text(area.get());
        // Should include left column content
        QVERIFY(text.contains(QStringLiteral("L1")));
        QVERIFY(text.contains(QStringLiteral("L2")));
        QVERIFY(text.contains(QStringLiteral("L3")));
        // Right column and footer are outside the geometric selection range,
        // so they shouldn't be included regardless of block logic
    }
}

void TextPageBlockTest::testCrossBlockSelectionLinePrecision()
{
    // Cross-block selection should not pull in the previous line when
    // lines are closely spaced (avoid "same line" false positives).

    Okular::Page page(0, 1000, 1000, Okular::Rotation0);

    Okular::TextEntity::List words;
    // Two tightly spaced lines in the left block
    words.append(Okular::TextEntity(QStringLiteral("L1"), Okular::NormalizedRect(0.1, 0.100, 0.2, 0.110)));
    words.append(Okular::TextEntity(QStringLiteral("L2"), Okular::NormalizedRect(0.1, 0.119, 0.2, 0.129)));
    // One line in the right block
    words.append(Okular::TextEntity(QStringLiteral("R1"), Okular::NormalizedRect(0.6, 0.150, 0.7, 0.160)));

    Okular::TextPage *textPage = new Okular::TextPage(words);

    QList<Okular::LayoutBlock> blocks;
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("left"), 0,
        Okular::NormalizedRect(0.0, 0.0, 0.45, 1.0),
        QStringLiteral("TEXT"), 0, 1.0));
    blocks.append(Okular::LayoutBlock(
        QStringLiteral("right"), 0,
        Okular::NormalizedRect(0.55, 0.0, 1.0, 1.0),
        QStringLiteral("TEXT"), 1, 1.0));

    textPage->setLayoutBlocks(blocks);
    page.setTextPage(textPage);

    // Start inside L2, end inside R1 to trigger cross-block selection.
    {
        Okular::TextSelection selection(
            Okular::NormalizedPoint(0.10, 0.124),  // Within L2 (left edge to catch same-line tolerance)
            Okular::NormalizedPoint(0.65, 0.155)); // Within R1
        std::unique_ptr<Okular::RegularAreaRect> area = textPage->textArea(selection, false);
        QVERIFY(area);

        QString text = textPage->text(area.get());
        QVERIFY(!text.contains(QStringLiteral("L1"))); // Should not pull previous line
        QVERIFY(text.contains(QStringLiteral("L2")));
        QVERIFY(text.contains(QStringLiteral("R1")));
    }
}

QTEST_MAIN(TextPageBlockTest)
#include "textpageblocktest.moc"
