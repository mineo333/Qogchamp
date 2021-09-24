This is a simple kernel library that helps do what I call Ghost File Operations (GhostFops for short). It does this by writing to pages in the page cache. This works because for some reason the CPU won't dirty the page. IDK why. PLS HELP ME. I'M LITERALLY LOSING MY SANITY.

NOTES: kmap is not the problem here. In 64 bit kmap literally just gets the virtual address that the page is mapped to (Because no pages are in low memory)
