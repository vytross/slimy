# slimy
An attempt at making a blazingly fast slime cluster finder. Implements some simple GPU parallelization for a roughly 5X speedup, but it's still pretty fast even in CPU-only mode.

Currently this is the only searching tool that I've come across that actually searches across varying y-levels for the true optimal spawning spaces. On the technicality that it searches a 25X greater volume than everyone else, this is *technically* the fastest slime searcher ever made B).

Still a WIP; many improvements to be made to the UI and some potentially algorithmically too, if I can think of more stuff. I'm sure I will.

Estimated whole-world search time: 
~~10 months~~ 
~~6 months~~
~~40 days~~
~~7 days~~
9.5 hours

### TODO
- Add simple GUI for easier use
- Increase speed
- Turn it into a proper CMake setup so people can actually compile it themselves
- Add version control for better height resolution
- Increase speeed
- Add edge case detection (mushroom islands, deep dark in newer versions)
- Add pack spawning score mechanic
- Increase speed
