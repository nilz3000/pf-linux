.. SPDX-License-Identifier: GPL-2.0
.. _proactive_compaction:

====================
Proactive Compaction
====================

Many applications benefit significantly from the use of huge pages.
However, huge-page allocations often incur a high latency or even fail
under fragmented memory conditions. Proactive compaction provides an
effective solution to these problems by doing memory compaction in the
background.

The process of proactive compaction is controlled by a single tunable:

        /sys/kernel/mm/compaction/proactiveness

This tunable takes a value in the range [0, 100] with a default value of
20. This tunable determines how aggressively compaction is done in the
background. Setting it to 0 disables proactive compaction.

Note that compaction has a non-trivial system-wide impact as pages
belonging to different processes are moved around, which could also lead
to latency spikes in unsuspecting applications. The kernel employs
various heuristics to avoid wasting CPU cycles if it detects that
proactive compaction is not being effective.
