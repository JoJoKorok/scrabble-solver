# Bundled dictionary

`enable2k.txt` is derived from the ENABLE 2K master word list, a North
American English list designed for word games. ENABLE is an open alternative
to proprietary official Scrabble dictionaries; it is not the current NASPA
Word List or Collins Scrabble Words list.

## Source and attribution

The source archive was retrieved from Bart Massey's word-list mirror at
revision `af52415c13af809bd8757a40f17f46e79d09583c`:

- archive: `enable2k.txt.gz`;
- archive SHA-256:
  `2c1093669cd16439bdb0a693a0058626c9c9f82e59244c9b0bde89515d44d3ad`;
- original entries: 173,528; and
- original ENABLE 2K documentation:
  <https://github.com/BartMassey/wordlists/blob/af52415c13af809bd8757a40f17f46e79d09583c/README-enable2k.txt>.

ENABLE was compiled by Mendel Cooper with major word-list research and
contributions from Alan Beale and other members of the word-game community.
The original documentation formally releases the ENABLE master word list into
the public domain and asks applications using it to credit its source and
originators.

## Transformation

The source file was transformed deterministically for the solver:

1. retain only entries matching `[A-Za-z]{1,15}`;
2. convert every entry to uppercase;
3. sort entries alphabetically; and
4. remove duplicate entries.

The result contains 169,266 words. Its SHA-256 is
`452528054b24cce68199a01534e4639d451ac0da3bdac70113546ad6e13fda0e`.

The 4,262 excluded source entries are longer than the solver's 15-letter
limit or contain characters outside unaccented ASCII letters. No vocabulary
judgments were added during this transformation.
