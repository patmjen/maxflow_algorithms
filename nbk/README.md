# Maxflow with large graph support
Modified version of Maxflow algorithm by Yuri Boykov and Vladimir Kolmogorov for very large graphs. Original source code availbable at http://pub.ist.ac.at/~vnk/software.html.

## Original description
An implementation of the maxflow algorithm described in:

<ul><b>An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in Computer Vision. </b><br>
Yuri Boykov and Vladimir Kolmogorov.<br>
<em>In IEEE Transactions on Pattern Analysis and Machine Intelligence, September 2004.</em>
</ul>

## Modifications
Changed to allow for very large graphs with up to 2.1 billion nodes and "any number" of edges/terms.

Main changes include:
- Changed edge variables to `long long`.
- Changed a few other variables to `long long`.
- Reduced `sizeof(node)` and `sizeof(arc)` by changing some variable types in the `struct`s.
