#!/usr/bin/env python3
"""
BPE (Byte Pair Encoding) text compressor for Akalabeth ROM texts.

Optimizations vs. naive BPE:
  1. Character remapping  -- only chars that actually appear are literals;
                             the freed token space becomes extra BPE rules.
  2. Weighted pair scoring -- score = raw_count * (depth_a + depth_b - 1)
                              biases toward merging already-composed tokens,
                              producing better long-pattern coverage.

Generates:
  assets/bpe_table.h  -- remap table + merge-rule table declarations
  assets/bpe_table.c  -- definitions
  assets/bpe_texts.h  -- compressed text declarations
  assets/bpe_texts.c  -- compressed text definitions

Usage (run from project root):
  python tools/bpe_compress.py
"""

import os
from collections import Counter

# ---------------------------------------------------------------------------
# Text definitions  (identifier, raw Python string)
# ---------------------------------------------------------------------------
TEXTS = [
    # ── views.c ──────────────────────────────────────────────────────────
    ("BPE_EXPLAIN",
        "\nThe Players Stat's:\n\n\n"
        "Hit Points- Amount of Damage a\n"
        "            Player can absorb\n"
        "            before Death\n"
        "Strength--- Related to Damage\n"
        "            Inflicted by Player\n"
        "            against Monsters.\n"
        "Dexterity-- Related to the \n"
        "            Probability of a\n"
        "            hitting a Monster\n"
        "Stamina---- Related of Player\n"
        "            Defense against\n"
        "            Monsters\n"
        "Wisdom----- This Attribute is\n"
        "            used in Special\n"
        "            (Quest!) Routines\n"
        "Gold------- Money!!\n"
        "            Cash!!\n"
        "            Assets!!"
    ),
    ("BPE_EXPLAIN2",
        "\nThe Towns and Buying Items:\n\n"
        "To buy any item one need only\n"
        "type the first letter of the\n"
        "item wanted. The cost of the\n"
        "respective items is displayed\n"
        "while in the town.\n"
        "The Game is started in a town\n"
        "somewhere on the 20x20 map.\n\n\n\n"
        "        Fighters and Magi\n"
        "The disadvantage of being a\n"
        "fighter is the lack of the\n"
        "ability to control the magic\n"
        "amulet, whereas magi can not use\n"
        "rapiers or bows."
    ),
    ("BPE_STORY",
        "\n\n  Many, many, many years ago the\n"
        "Dark Lord Mondain, Archfoe of\n"
        "British, traversed the lands of\n"
        "Akalabeth spreading evil and\n"
        "death as he passed.\n"
        "\n\nBy the time Mondain was driven\n"
        "from the land by British, bearer\n"
        "of the White Light, he had done\n"
        "much damage unto the lands.\n"
        "\n\n'Tis thy duty to help rid\n"
        "Akalabeth of the foul beasts\n"
        "which infest it, while trying\n"
        "to stay alive!!!"
    ),
    ("BPE_CONTROLS",
        "\nMovement:\n"
        "\n\n\n"
        "-Key-  Outdoors        Dungeon\n"
        "--------------------------------\n\n"
        " UP    Move North   Move Forward\n\n"
        "LEFT   Move West    Turn Left\n\n"
        "RIGHT  Move East    Turn Right\n\n"
        "DOWN   Move South   Turn Around\n\n"
        "PAUSE   Status       Status/Map\n\n"
        "BTN1    N/A           Attack\n\n"
        "BTN2  Go into Town Climb Ladder"
    ),
    ("BPE_APPENDIX",
        "\n\n  Thou doest know the basics of\n"
        "the game, experiment with the\n"
        "commands.\n"
        "There is much left unsaid for\n"
        "thee to discover in the future..\n"
        "\nGo now unto the world and seek\n"
        "adventure where thou might!!!\n"
        "P.S.-Search out the Castle of\n"
        "Lord British, Use the -E- Key\n"
        "to go in!"
    ),

    # ── engine2.c ─────────────────────────────────────────────────────────
    ("BPE_ASSIGN_QUEST",
        "\nGO NOW UPON THIS QUEST, AND MAY\n"
        "LADY LUCK BE FAIR UNTO YOU...\n"
        "ALSO I, BRITISH, HAVE INCREASED\n"
        "EACH OF THY ATTRIBUTES BY ONE!\n"
        "\n"
    ),
    ("BPE_CASTLE_WELCOME",
        "\nWELCOME PEASANT INTO THE HALLS\n"
        "OF THE MIGHTY LORD BRITISH.\n"
        "HEREIN THOU MAYCHOOSE TO DARE\n"
        "BATTLE WITH THE EVIL\n"
        "CREATURES OF THE DEPTHS,\n"
        "FOR GREAT REWARD!\n\n"
    ),
    ("BPE_CASTLE_FIRST_QUEST",
        "\nGOOD! THOU SHALT TRY TO BECOME A\n"
        "KNIGHT!!!\n"
        "THY FIRST TASK IS TO GO INTO THE\n"
        "DUNGEONS AND TO RETURN ONLY\n"
        "AFTER KILLING A(N) "
    ),
    ("BPE_CASTLE_RETURN",
        " WHY HAST THOU RETURNED?\n"
        "THOU MUST KILL A(N) "
    ),
    ("BPE_QUEST_GO",
        "\nGO NOW AND COMPLETE THY QUEST!\n\n"
        "       PRESS A BUTTON TO CONT."
    ),
    ("BPE_QUEST_DONE",
        "\nTHOU HAST ACOMPLISHED THY QUEST!\n"
    ),
    ("BPE_QUEST_WORTHY",
        "\nTHOU HAST PROVED THYSELF WORTHY"
        "OF KNIGHTHOOD, CONTINUE PLAY IF THOU"
        "DOTH WISH, BUT THOU HAST ACOMPLISHED\n"
        "THE MAIN OBJECTIVE OF THIS GAME...\n"
    ),
    ("BPE_QUEST_CALIFORNIA",
        "\n...CALL CALIFORNIA PACIFIC COMPUTER\n"
        "AT (415)-569-9126 TO REPORT THIS\n"
        "AMAZING FEAT!\n"
    ),
    ("BPE_QUEST_FOOLHEARTY",
        "\n   NOW MAYBE THOU ART FOOLHEARTY\n"
        "ENOUGH TO TRY DIFFICULTY LEVEL "
    ),
    ("BPE_QUEST_NOT_ENOUGH",
        "UNFORTUNATELY, THIS IS NOT ENOUGH TO\n"
        "BECOME A KNIGHT.\n"
    ),
    ("BPE_CASTLE_BEGONE",
        "\nTHEN LEAVE AND BEGONE!"
        "\n        CONTINUE."
    ),
    ("BPE_CASTLE_ADVENTURE",
        "\nDOEST THOU WISH FOR GRAND\n ADVENTURE ? "
    ),
    ("BPE_CHOOSE_LUCKY",
        "Choose thy lucky number:"
    ),
    ("BPE_LEVEL_OF_PLAY",
        "Level of Play:"
    ),
    ("BPE_PLAY_WITH_THESE",
        "Shallt thou play with these\nqualities?"
    ),
    ("BPE_FIGHTER_OR_MAGE",
        "\n\nShalt thou be a \n   Fighter or a  Mage?"
    ),
    ("BPE_NOW_KILL",
        "\nNOW THOU MUST KILL A(N) "
    ),
    ("BPE_STATS_WEAPONS",
        "\n   STAT'S          WEAPONS"
    ),
    ("BPE_SHOP_HEADER",
        "PRICE    DAMAGE     ITEM"
    ),
    ("BPE_SHOP_WELCOME",
        "WELCOME TO THE ADVENTURE SHOP"
    ),
    ("BPE_SHOP_BUY",
        "WHICH ITEM SHALT THOU BUY "
    ),
    ("BPE_AAHH",
        "\n\n\nAAHH!!......"
    ),

    # ── engine.c – static string constants ───────────────────────────────
    ("BPE_LEAVE_DUNGEON",   "LEAVE DUNGEON\n"),
    ("BPE_GO_UP_TO_LEVEL",  "GO UP TO LEVEL "),
    ("BPE_THOU_HAST_GAINED","THOU HAST GAINED\n"),
    ("BPE_HIT_POINTS",      " HIT POINTS\n"),
    ("BPE_BEEN_TURNED",     "YOU HAVE BEEN TURNED\n"),
    ("BPE_TITLE",           "WELCOME TO AKALABETH"),
    ("BPE_SUBTITLE",        "WORLD OF DOOM!"),
    ("BPE_YOU_CANT_PASS",   "YOU CAN'T PASS\nTHE MOUNTAINS\n"),
    ("BPE_NOT_OWNED",       "NOT OWNED\n"),
    ("BPE_COMMAND_PROMPT",  "COMMAND? "),

    # ── engine.c – inline print() calls ──────────────────────────────────
    ("BPE_BEING_ATTACKED",  "YOU ARE BEING ATTACKED\n"),
    ("BPE_GREMLIN_FOOD",    "A GREMLIN STOLE SOME FOOD"),
    ("BPE_THIEF_STOLE",     "A THIEF STOLE A "),
    ("BPE_TRAP",            "AAARRRGGGHHH!!! A TRAP!\n"),
    ("BPE_GOLD",            "GOLD!!!!!\n"),
    ("BPE_PIECES_SUFFIX",   "-PIECES OF EIGHT\n"),
    ("BPE_MONSTER_HP",      "'S HIT POINTS="),
    ("BPE_THOU_KILLED",     "THOU HAST KILLED A "),
    ("BPE_THOU_RECEIVE",    "\nTHOU SHALT RECEIVE\n"),
    ("BPE_PIECES",          " PIECES OF EIGHT"),
    ("BPE_CHOOSE_WEAPON",   "CHOOSE WEAPON:"),
    ("BPE_NO_RAPIER",       "MAGES CAN'T USE RAPIERS!\n"),
    ("BPE_NO_BOW",          "MAGES CAN'T USE BOWS!\n"),
    ("BPE_AMULET_CHOOSE",   "MAGIC AMULET - CHOOSE:"),
    ("BPE_LAST_CHARGE",     "LAST CHARGE ON THIS AMULET!\n"),
    # "1-LADDER-UP", "2-LADDER-DN", "LADDER UP\n", "LADDER DOWN\n":
    # too short to compress — keep as plain print() in engine.c
    ("BPE_MAGIC_ATTACK",    "MAGIC ATTACK\n"),
    ("BPE_INTO_TOAD",       "INTO A TOAD!\n"),
    ("BPE_INTO_LIZARD",     "INTO A LIZARD MAN\n"),
    ("BPE_STARVED",         "\nYOU HAVE STARVED!!!!!\n"),
    ("BPE_VISIT_BRITISH",   " VISIT LORD BRITISH!"),
    ("BPE_RETURN_BRITISH",  " RETURN TO LORD BRITISH!"),
    ("BPE_TURN_RIGHT",      "TURN RIGHT\n"),
    ("BPE_TURN_LEFT",       "TURN LEFT\n"),
    ("BPE_TURN_AROUND",     "TURN AROUND\n"),
    ("BPE_CREDITS_1",       "Reprogammed by DarkTrym"),
    ("BPE_CREDITS_2",       "SMS Competition 2026"),
    ("BPE_CREDITS_3",       "supported by DevKitSMS & PSGlib"),
]

BPE_END = 0xFF

# ---------------------------------------------------------------------------
# Optimization 1: character remapping
# ---------------------------------------------------------------------------

def build_remap(strings):
    """
    Find every byte that appears in the corpus and assign dense IDs.

    Returns:
      encode  -- dict: original_byte -> remapped_id
      remap   -- list: remapped_id -> original_byte  (the C table)
    """
    unique = sorted(set(b for s in strings for b in s.encode('latin-1')))
    encode = {c: i for i, c in enumerate(unique)}
    return encode, unique          # unique[id] == original byte


# ---------------------------------------------------------------------------
# Optimization 2: weighted BPE
# ---------------------------------------------------------------------------

def bpe_compress(strings, encode, num_literals, max_rules):
    """
    BPE with weighted pair scoring.

    score(A, B) = raw_count(A,B) * (depth(A) + depth(B) - 1)

    depth(literal) = 1
    depth(BPE_token) = depth(left_child) + depth(right_child)

    Pairs with raw_count < 2 are excluded (no actual saving possible).
    """
    # Encode all strings to remapped byte sequences
    seqs = [[encode[b] for b in s.encode('latin-1')] for s in strings]

    rules  = []
    depths = {i: 1 for i in range(num_literals)}
    next_tok = num_literals

    for _ in range(max_rules):
        if next_tok > 0xFE:
            break

        # Count raw occurrences and weighted scores simultaneously
        raw   = Counter()
        score = Counter()
        for seq in seqs:
            for i in range(len(seq) - 1):
                pair = (seq[i], seq[i + 1])
                raw[pair] += 1
                score[pair] += depths[seq[i]] + depths[seq[i + 1]] - 1

        # Only consider pairs that genuinely appear more than once
        candidates = {p: score[p] for p, c in raw.items() if c >= 2}
        if not candidates:
            break

        best = max(candidates, key=candidates.get)
        tok  = next_tok
        next_tok += 1
        rules.append(best)
        depths[tok] = depths[best[0]] + depths[best[1]]

        # Apply the merge to every sequence
        for seq in seqs:
            i, new_seq = 0, []
            while i < len(seq):
                if i < len(seq) - 1 and seq[i] == best[0] and seq[i + 1] == best[1]:
                    new_seq.append(tok)
                    i += 2
                else:
                    new_seq.append(seq[i])
                    i += 1
            seq[:] = new_seq

    return rules, seqs, depths


# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------

def char_label(b, remap):
    """Human-readable label for a remapped id or raw byte."""
    raw = remap[b] if b < len(remap) else None
    if raw is not None:
        if raw == 0x0A: return "'\\n'"
        if raw == 0x20: return "' '"
        if 0x21 <= raw < 0x7F: return f"'{chr(raw)}'"
        return f"0x{raw:02X}"
    return f"T{b - len(remap)}"


def write_table_h(remap, rules, path):
    nl = len(remap)
    nr = len(rules)
    with open(path, 'w', newline='\n') as f:
        f.write(f"""\
#ifndef BPE_TABLE_H
#define BPE_TABLE_H

#ifdef __cplusplus
extern "C" {{
#endif

/*
 * BPE decoder tables (generated by tools/bpe_compress.py).
 *
 * Encoding:
 *   0 .. BPE_NUM_LITERALS-1   literal ids  -> bpe_remap[id] gives the real char
 *   BPE_NUM_LITERALS .. 0xFE  BPE tokens   -> bpe_table[tok-BPE_NUM_LITERALS]
 *   0xFF                      end of string
 */
#define BPE_NUM_LITERALS  {nl}U
#define BPE_NUM_RULES     {nr}U
#define BPE_TOKEN_BASE    BPE_NUM_LITERALS
#define BPE_END_MARKER    0xFFU

/* bpe_remap[id] == the actual ASCII/Latin-1 byte for literal id */
extern const unsigned char bpe_remap[BPE_NUM_LITERALS];

/* bpe_table[i] == {{left_child, right_child}} for token (BPE_NUM_LITERALS + i) */
extern const unsigned char bpe_table[BPE_NUM_RULES][2];

#ifdef __cplusplus
}}
#endif

#endif /* BPE_TABLE_H */
""")


def write_table_c(remap, rules, path):
    nl, nr = len(remap), len(rules)
    with open(path, 'w', newline='\n') as f:
        f.write('#include "bpe_table.h"\n\n')

        # Remap table
        f.write('/* literal id -> original byte */\n')
        f.write(f'const unsigned char bpe_remap[{nl}] = {{\n')
        row_size = 16
        for row_start in range(0, nl, row_size):
            row = remap[row_start:row_start + row_size]
            labels = ', '.join(f'0x{b:02X}' for b in row)
            f.write(f'    {labels},\n')
        f.write('};\n\n')

        # Merge table
        f.write('/* token (BPE_NUM_LITERALS+i) expands to {left, right} */\n')
        f.write(f'const unsigned char bpe_table[{nr}][2] = {{\n')
        for i, (a, b) in enumerate(rules):
            tok = nl + i
            la, lb = char_label(a, remap), char_label(b, remap)
            f.write(f'    {{0x{a:02X}, 0x{b:02X}}},  /* 0x{tok:02X}: {la}+{lb} */\n')
        f.write('};\n')


def write_texts_h(text_defs, path):
    with open(path, 'w', newline='\n') as f:
        f.write("""\
#ifndef BPE_TEXTS_H
#define BPE_TEXTS_H

#ifdef __cplusplus
extern "C" {
#endif

""")
        for name, _ in text_defs:
            f.write(f'extern const unsigned char {name}[];\n')
        f.write("""\

#ifdef __cplusplus
}
#endif

#endif /* BPE_TEXTS_H */
""")


def write_texts_c(text_defs, compressed, path):
    with open(path, 'w', newline='\n') as f:
        f.write('#include "bpe_texts.h"\n\n')
        for (name, orig), comp in zip(text_defs, compressed):
            o = len(orig.encode('latin-1'))
            c = len(comp) + 1   # +1 for end marker
            pct = 100 * c // o if o else 0
            f.write(f'/* {name}: {o} -> {c} bytes ({pct}%) */\n')
            f.write(f'const unsigned char {name}[] = {{\n')
            all_bytes = list(comp) + [BPE_END]
            for row_start in range(0, len(all_bytes), 16):
                row = all_bytes[row_start:row_start + 16]
                f.write('    ' + ', '.join(f'0x{b:02X}' for b in row) + ',\n')
            f.write('};\n\n')


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    script_dir  = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(script_dir)
    assets_dir  = os.path.join(project_dir, 'assets')

    strings = [text for _, text in TEXTS]

    # Build the character remap
    encode, remap = build_remap(strings)
    num_literals  = len(remap)
    max_rules     = 0xFE - num_literals   # use every available token slot

    print(f"Corpus: {len(strings)} texts, "
          f"{sum(len(s.encode('latin-1')) for s in strings)} bytes raw")
    print(f"Unique chars: {num_literals}  "
          f"-> BPE token slots: {max_rules}  (was 127 without remap)")
    print(f"Compressing with weighted scoring ...")

    rules, compressed, depths = bpe_compress(strings, encode, num_literals, max_rules)
    print(f"Generated {len(rules)} merge rules.\n")

    total_orig = sum(len(s.encode('latin-1')) for s in strings)
    total_comp = sum(len(c) + 1 for c in compressed)
    table_bytes = num_literals + len(rules) * 2   # remap + merge table
    saving      = total_orig - total_comp - table_bytes

    print(f"Text:          {total_orig:5d} -> {total_comp:5d} bytes "
          f"({100 * total_comp // total_orig}%)")
    print(f"Table overhead: {table_bytes:4d} bytes  "
          f"(remap={num_literals}, rules={len(rules)*2})")
    print(f"Net saving (before decoder code): {saving} bytes\n")

    for (name, orig), comp in zip(TEXTS, compressed):
        o = len(orig.encode('latin-1'))
        c = len(comp) + 1
        exp = depths.get(comp[0], 1) if comp else 0
        print(f"  {name:<30} {o:4d} -> {c:4d} bytes ({100*c//o:3d}%)")

    print()
    write_table_h(remap, rules, os.path.join(assets_dir, 'bpe_table.h'))
    write_table_c(remap, rules, os.path.join(assets_dir, 'bpe_table.c'))
    write_texts_h(TEXTS,            os.path.join(assets_dir, 'bpe_texts.h'))
    write_texts_c(TEXTS, compressed, os.path.join(assets_dir, 'bpe_texts.c'))

    print("Written: assets/bpe_table.h")
    print("Written: assets/bpe_table.c")
    print("Written: assets/bpe_texts.h")
    print("Written: assets/bpe_texts.c")


if __name__ == '__main__':
    main()
