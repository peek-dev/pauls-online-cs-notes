# Setting up the example

Let's take an example `va = 0x0123456789abcdef`. This is larger than what we'll actually need, so we will have to isolate the VPN segments and POBITS accordingly while ignoring some of the highest-order bits.
* The page size is $$2^{POBITS}$$ bytes, which in this case is $$2^8 = 256$$ bytes.
    * There are $$256 / 8$$ page table entries _per page_ since a pagetable entry is 8 bytes (a constant size given in the writeup)--producing 32 page table entries per page.
    * The number of bits needed to address all entries in one page is $$\log_2{32} = 5$$ bits, so each VPN segment will be 5 bits.

It's best here to go from least-significant to most-significant in how we parse the virtual address.
* `POBITS = 8`, so the lowest-order 8 bits `0xef = 0b 1110 1111` constitute the _page offset_. We'll save this for later--namely, the very last step of the multi-level translation
* We'll read the next-highest 30 bits since `LEVELS = 6` and the number of bits per VPN segment is 5. A binary conversion is helpful here:
```
0x0123456789abcdef
= 0b 0000 0001 0010 0011 0100 0101 01 | [10 011] | [1 1000] | [1001 1] | [010 10] | [11 110] | [0 1101] | [1110 1111]
UNUSED | [VPN segment 1] | [VPN segment 2] | [VPN segment 3] | [VPN segment 4] | [VPN segment 5] | [VPN segment 6] | [OFFSET]
```
Note: spacing here is maintained in groupings of 4 and reconciled when reading out the actual values for VPN segments.

* So, the VPN segments are as follows:
    * VPN segment 1 = `0b 1 0011` = `0x13` (not `0x91`, since hex digits are grouped least-significant to most-significant).
    * VPN segment 2 = `0b 1 1000` = `0x18`
    * VPN segment 3 = `0b 1001 1` = `0x13` 
    * VPN segment 4 = `0b 010 10` = `0x0a` = `0xa`.
    * VPN segment 5 = `0b 11 110` = `0x1e`
    * VPN segment 6 = `0b 0 1101` = `0x0d` = `0xd`

# Translation steps

We'll first consult the `ptbr`. Let's say we've already allocated a top-level pagetable so that PTBR holds a valid page-aligned address of `0x100` (page aligned since integer multiple of the page size, which is `256 = 0x100`).

## First-level translation

1. We'll use our first VPN segment to index off of the base of the page we're currently looking at, which is the same for this first level as PTBR. We'll need to add the segment 1 `0x13`, multiplied by the pagetable entry size, to the base address of the current page (something like `0x100 + (0x13 * 0x8)` or equivalent). Multiplication gets rather dicey, so if we consider a pagetable as an _array_ of pagetable entries, we could also treat the current page as an array and index into our current page by the current VPN segment--this will handle the pointer arithmetic quite conveniently for us.
     * Let's say the PTE we retrieve at address `0x100 + (0x13 * 0x8) = 0x198` looks like the following (full 64 bits represented): `0x 0000 0000 0098 7601`. We can tell since this number is odd that the valid bit is set to indicate a valid next-level mapping. The PPN is _everything else_ that is not in the POBITS lowest-order bits, which will be `0x 0000 0000  0098 76` = `0x9876`. Aligning this to the page size produces address `0x987600`, which we now consider this our base address for the next level of the translate.

## Second-level translation

2. We'll use our second VPN segment to index off of the base of the page we're currently looking at (base address `0x987600`). We'll use segment 2 `0x18` to index within the current page and retrieve a pagetable entry at address `0x987600 + (0x18 * 0x8) = 0x9876c0 `.
    * Let's say this PTE looks like the following (full 64 bits represented): `0x 0000 0000 ab12 ef01`. As before, the number the PTE stores is odd, and the PTE therefore indicates that a valid next-level mapping exists. We'll consider the PPN to be everything more significant than the POBITS bits, which means the next-level PPN is `0xab12ef`. Aligning this to the page size means the base address of our next-level page will be `0xab12ef00`.

## Third-level translate

3. We'll use our third VPN segment to index off of the base of the page we're currently looking at (base address `0xab12ef00`). We'll use segment 3 `0x13` to index within the current page and retrieve a pagetable entry at address `0xab12ef00 + (0x13 * 0x8) = 0xab12ef98`.
    * Let's say this PTE looks like (full 64 bits again): `0x 0000 0000 0019 8601`. Like before, our PTE's stored number is odd to indicate the least significant bit (the valid bit) is set and a next-level physical mapping exists. By the same procedure before, our PPN is everything more significant than the POBITS bits = `0x1986` and aligning this to the page size produces next-level page base address = `0x198600`.

## Fourth-level translate

4. We'll use our fourth VPN segment to index off of the base of the page we're currently looking at (base address `0x198600`). We'll use segment 4 `0xa` to index off of the current page and retrieve a PTE at address `0x198600 + (0xa * 0x8) = 0x198680`.
    * Let's say this PTE looks like (full 64 bits): `0x 0000 0001 72b4 e901`. The valid bit is set, a next-level physical mapping exists, and the next-level PPN is everything more significant: `0x172b4e9`. Aligning this to the page size produces next-level page base address `0x172b4e900`.

## Fifth-level translate

5. We'll use our fifth VPN segment to index off of the base of the page we're currently looking at (base address `0x172b4e900`). We'll use segment 5 `0x1e` to index within the current page and retrieve a PTE at address `0x172b4e900 + (0x1e * 0x8) = 0x172b4e9f0`.
    * Let's say this PTE looks like (full 64 bits): `0x 0000 0000 0092 00ff`. This is an odd-looking PTE that we ideally should change to a friendlier format (like the PTEs before). I'm including this example to illustrate that our valid bit is still set, and since we don't care about the rest of the POBITS bits contents, we can simply retrieve the PPN like before as everything more significant than POBITS: next-level PPN is `0x9200` since POBITS is `0xff`, and the valid bit is set. Aligning the PPN `0x9200` to the page size gets next-level page base address `0x920000`.

## Sixth-level translate

6. We'll use our sixth VPN segment to index off of the base of the page we're currently looking at (base address `0x920000`). We'll use segment 6 `0xd` to index within the current page and retrieve a PTE at address `0x920000 + (0xd * 0x8) = 0x920068`. 
    * Let's say this PTE looks like (full 64 bits): `0x 0000 0000 0067 8901`. We're returning to a friendlier PTE format with a set valid bit and a PPN that's everything more significant (`0x6789`). Aligning this to the page size produces a **base address of the final physical page** of `0x678900`. We know this is the final physical page base address because we have completed all levels of our translation as we previously defined LEVELS. 

# Getting the final physical address

**Lastly**, we "concatenate" or "put together" the final physical page number with the page offset we had saved for later. **This gets us our final physical address that we should return:** `0x6789ef`.