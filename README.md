This is the first SVN checkout of V12 / TorqueGameEngine. It represents Tribes 2 at an earlier state in it's development.

The largest obvious removal relates to WON authentication data. There's a lot of code in the engine dedicated to the pre-connect handshake that isn't present here.

Several NetObject and NetEvent classes in Tribes 2 are missing from this source code release.
The objects that do exist are in an intermediate state - bug fixes relating to unpacking network data are inconsistently applied. The final Tribes 2 release still has unpacking bugs, but fewer.

It's also vaugely interesting that there is evidence of a FogChallengeEvent. I can't find any usage in demo recordings, so it doesn't seem to be triggered by release builds.
