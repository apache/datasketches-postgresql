Module for PostgreSQL to support approximate algorithms based on the Datasketches core library sketches-core-cpp.
See https://datasketches.github.io/ for details.

This module currently supportstwo sketches:

- CPC (Compressed Probabilistic Counting) sketch - very compact (when serialized) distinct-counting sketch
- KLL float quantiles sketch - for estimating distributions: quantile, rank, PMF (hystogram), CDF

Examples:

Distinct counting:

Exact count distinct:

	$ time psql test -c "select count(distinct id) from random_ints_100m;"
	  count
	----------
	 63208457
	(1 row)
	
	real	1m59.060s

Approximate count distinct:

	$ time psql test -c "select cpc_sketch_distinct(id) from random_ints_100m;"
	 cpc_sketch_distinct 
	---------------------
	    62716231.1448033
	(1 row)
	
	real	0m21.811s

Note that the above one-off distinct count is just to show the basic usage. Most importantly, the sketch can be used as an "additive" disctinct count metric in a data cube.

Estimating quanitles and ranks:

Table "normal" has 1 million values from the normal distribution mean=0 and stddev=1
We can build a sketch, which represents the distribution (create table kll\_float\_sketch\_test(sketch kll\_float\_sketch)):

	$ psql test -c "insert into kll_float_sketch_test select kll_float_sketch_build(value) from normal";
	INSERT 0 1

We expect the value with rank 0.5 (median) to be approximately 0:

	$ psql test -c "select kll_float_sketch_get_quantile(sketch, 0.5) from kll_float_sketch_test";
	 kll_float_sketch_get_quantile 
	-------------------------------
	                    0.00648344

In reverse: we expect the rank of value 0 (true median) to be approximately 0.5:

	$ psql test -c "select kll_float_sketch_get_rank(sketch, 0) from kll_float_sketch_test";
	 kll_float_sketch_get_rank 
	---------------------------
	                  0.496289

Note that the normal distribution was used just to show the basic usage. The sketch does not make any assumptions about the distribution.
