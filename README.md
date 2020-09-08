<!--
    Licensed to the Apache Software Foundation (ASF) under one
    or more contributor license agreements.  See the NOTICE file
    distributed with this work for additional information
    regarding copyright ownership.  The ASF licenses this file
    to you under the Apache License, Version 2.0 (the
    "License"); you may not use this file except in compliance
    with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing,
    software distributed under the License is distributed on an
    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
    KIND, either express or implied.  See the License for the
    specific language governing permissions and limitations
    under the License.
-->
# PostgreSQL Adaptor for C++ DataSketches

Please visit the main [DataSketches website](https://datasketches.apache.org) for more information. 

If you are interested in making contributions to this site please see our [Community](https://datasketches.apache.org/docs/Community/) page for how to contact us.

---

This module currently supports the following sketches:

- CPC (Compressed Probabilistic Counting) sketch - very compact (smaller than HLL when serialized) distinct-counting sketch
- Theta sketch - distinct counting with set operations (intersection, a-not-b)
- HLL sketch - very compact distinct-counting sketch based on HyperLogLog algorithm
- KLL float quantiles sketch - for estimating distributions: quantile, rank, PMF (histogram), CDF
- Frequent strings sketch - capture the heaviest items (strings) by count or by some other weight

## How to build and install

This code is intended to be distributed as a PostgreSQL extension on [PGXN site](https://pgxn.org/)

This code is expected to be compatible with PostgreSQL versions 9.4 and higher. It was tested with REL\_11\_STABLE branch.
PostreSQL must be installed to compile the extension. The path to PostgreSQL executables must be set up (try running 'pg_config' to test).
For PostgreSQL installation instructions see [PostgreSQL documentation](https://www.postgresql.org/docs/current/tutorial-start.html)

This code requires C++11. It was tested with GCC 4.8.5 (standard in RedHat at the time of this writing), GCC 8.2.0, GCC 9.2.0, Apple LLVM version 10.0.1 (clang-1001.0.46.4) and version 11.0.0 (clang-1100.0.33.8).

This code depends on [datasketches-cpp version 1.0.0-incubating](https://github.com/apache/incubator-datasketches-cpp/releases/tag/1.0.0-incubating)

There are two slightly different ways to build this extension: from a PGXN distribution or from two separate packages: datasketches-postgresql and datasketches-cpp (either from GitHub or from [Apache archive](http://archive.apache.org/dist/incubator/datasketches/))

### PGXN extension

   - Download the datasketches extension from [PGXN](https://pgxn.org/dist/datasketches/)
   - Unzip the package (the core library datasketches-cpp is included)

### GitHub or Apache archive

   - Clone or download from GitHub or download from Apache archive both the datasketches-postgresql code and the core library datasketches-cpp (version mentioned above)
   - Place the core library as a subdirectory (or a link to it) inside of the datasketches-postgresql like so:
      - datasketches-cpp
      - datasketches-postgresql
          - datasketches-cpp -> ../datasketches-cpp

### Building and installing

   - make
   - sudo make install

On MacOSX Mojave, if you see a warning like this:<br>
clang: warning: no such sysroot directory: ‘/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk’ [-Wmissing-sysroot]<br>
and the compilation fails because of not being able to find system include files, this is a known OSX problem. There are known solutions on the Internet.

### Verifying installation with a test database

   - Make sure that PostgreSQL is running. For instance, using Homebrew on MacOSX, start the service:
     - brew services start postgresql
   - Create a test database if it does not exist yet (on the command line):
      - createdb test
   - Run the client (console) using the test database:
      - psql test
   - Create datasketches extension in the test database:
      - create extension datasketches;
   - Try some of the datasketches functions:
      - select cpc\_sketch\_to\_string(cpc\_sketch\_build(1));

You should see the following result:

	       cpc_sketch_to_string        
	-----------------------------------
	 ### CPC sketch summary:          +
	    lgK            : 11           +
	    seed hash      : 93cc         +
	    C              : 1            +
	    flavor         : 1            +
	    merged         : false        +
	    compressed     : false        +
	    intresting col : 0            +
	    HIP estimate   : 1            +
	    kxp            : 2047.5       +
	    offset         : 0            +
	    table          : allocated    +
	    num SV         : 1            +
	    window         : not allocated+
	 ### End sketch summary           +
	 
	(1 row)

### Docker

Build Docker image:

	$ docker build . -t datasketch-postgres:latest

Build Docker image with specific version

	$ docker build --pull --build-arg BASE_IMAGE_VERSION=10 -t datasketch-postgres:10 .

Run container:
  
	$ docker run --name some-postgres -e POSTGRES_PASSWORD=mysecretpassword -d datasketch-postgres:latest

Test DataSketches in PostgreSQL:

	$ docker exec -it some-postgres psql -U postgres
	postgres=# SELECT cpc_sketch_get_estimate(cpc_sketch_union(respondents_sketch)) AS num_respondents, flavor
	  FROM (
	    SELECT
	      cpc_sketch_build(respondent) AS respondents_sketch,
	      flavor,
	      country
	    FROM (
	      SELECT * FROM (
	        VALUES (1, 'Vanilla', 'CH'),
	               (1, 'Chocolate', 'CH'),
	               (2, 'Chocolate', 'US'),
	               (2, 'Strawberry', 'US')) AS t(respondent, flavor, country)) as foo
	    GROUP BY flavor, country) as bar
	  GROUP BY flavor;

## Examples

### Distinct counting with CPC sketch

Suppose 100 million random integer values uniformly distributed in the range from 1 to 100M have been generated and inserted into a table

Exact count distinct:

	$ time psql test -c "select count(distinct id) from random_ints_100m"
	  count
	----------
	 63208457
	(1 row)
	
	real	1m59.060s

Approximate count distinct:

	$ time psql test -c "select cpc_sketch_distinct(id) from random_ints_100m"
	 cpc_sketch_distinct 
	---------------------
	    63423695.9451363
	(1 row)
	
	real	0m20.680s

Note that the above one-off distinct count is just to show the basic usage. Most importantly, the sketch can be used as an "additive" distinct count metric in a data cube.

Aggregate union:

	create table cpc_sketch_test(sketch cpc_sketch);
	insert into cpc_sketch_test select cpc_sketch_build(1);
	insert into cpc_sketch_test select cpc_sketch_build(2);
	insert into cpc_sketch_test select cpc_sketch_build(3);
	select cpc_sketch_get_estimate(cpc_sketch_union(sketch)) from cpc_sketch_test;
	 cpc_sketch_get_estimate
	-------------------------
	        3.00024414612919

Non-aggregate union:

	select cpc_sketch_get_estimate(cpc_sketch_union(cpc_sketch_build(1), cpc_sketch_build(2)));
	 cpc_sketch_get_estimate 
	-------------------------
	        2.00016277723359

### Distinct counting with Theta sketch

See above for the exact distinct count of 100 million random integers

Approximate distinct count:

	$ time psql test -c "select theta_sketch_distinct(id) from random_ints_100m"
	 theta_sketch_distinct 
	-----------------------
	      64593262.4373193
	(1 row)

	real	0m19.701s

Note that the above one-off distinct count is just to show the basic usage. Most importantly, the sketch can be used as an "additive" distinct count metric in a data cube.

Aggregate union:

	create table theta_sketch_test(sketch theta_sketch);
	insert into theta_sketch_test select theta_sketch_build(1);
	insert into theta_sketch_test select theta_sketch_build(2);
	insert into theta_sketch_test select theta_sketch_build(3);
	select theta_sketch_get_estimate(theta_sketch_union(sketch)) from theta_sketch_test;
	 theta_sketch_get_estimate 
	---------------------------
	                         3

Non-aggregate set operations:

	create table theta_set_op_test(sketch1 theta_sketch, sketch2 theta_sketch);
	insert into theta_set_op_test select theta_sketch_build(1), theta_sketch_build(1);
	insert into theta_set_op_test select theta_sketch_build(1), theta_sketch_build(2);

	select theta_sketch_get_estimate(theta_sketch_union(sketch1, sketch2)) from theta_set_op_test;
	 theta_sketch_get_estimate 
	---------------------------
	                         1
	                         2
	(2 rows)

	select theta_sketch_get_estimate(theta_sketch_intersection(sketch1, sketch2)) from theta_set_op_test;
	 theta_sketch_get_estimate 
	---------------------------
	                         1
	                         0
	(2 rows)

	select theta_sketch_get_estimate(theta_sketch_a_not_b(sketch1, sketch2)) from theta_set_op_test;
	 theta_sketch_get_estimate 
	---------------------------
	                         0
	                         1
	(2 rows)

### Distinct counting with HLL sketch

See above for the exact distinct count of 100 million random integers

Approximate distinct count:

	$ time psql test -c "select hll_sketch_distinct(id) from random_ints_100m"
	 hll_sketch_distinct 
	---------------------
	    63826337.5738399
	(1 row)

	real	0m19.075s

Note that the above one-off distinct count is just to show the basic usage. Most importantly, the sketch can be used as an "additive" distinct count metric in a data cube.

Aggregate union:

	create table hll_sketch_test(sketch hll_sketch);
	insert into hll_sketch_test select hll_sketch_build(1);
	insert into hll_sketch_test select hll_sketch_build(2);
	insert into hll_sketch_test select hll_sketch_build(3);
	select hll_sketch_get_estimate(hll_sketch_union(sketch)) from hll_sketch_test;
		 hll_sketch_get_estimate 
	-------------------------
	        3.00000001490116

Non-aggregate union:

	select hll_sketch_get_estimate(hll_sketch_union(hll_sketch_build(1), hll_sketch_build(2)));
	 hll_sketch_get_estimate 
	-------------------------
	        2.00000000496705

### Estimating quanitles, ranks and histograms with KLL sketch

Table "normal" has 1 million values from the normal (Gaussian) distribution with mean=0 and stddev=1.
We can build a sketch, which represents the distribution:

	create table kll_float_sketch_test(sketch kll_float_sketch);
	$ psql test -c "insert into kll_float_sketch_test select kll_float_sketch_build(value) from normal"
	INSERT 0 1

We expect the value with rank 0.5 (median) to be approximately 0:

	$ psql test -c "select kll_float_sketch_get_quantile(sketch, 0.5) from kll_float_sketch_test"
	 kll_float_sketch_get_quantile 
	-------------------------------
	                    0.00648344

In reverse: we expect the rank of value 0 (true median) to be approximately 0.5:

	$ psql test -c "select kll_float_sketch_get_rank(sketch, 0) from kll_float_sketch_test"
	 kll_float_sketch_get_rank 
	---------------------------
	                  0.496289

Getting several quantiles at once:

	$ psql test -c "select kll_float_sketch_get_quantiles(sketch, ARRAY[0, 0.25, 0.5, 0.75, 1]) from kll_float_sketch_test"
	          kll_float_sketch_get_quantiles
	--------------------------------------------------
	 {-4.72317,-0.658811,0.00648344,0.690616,4.91773}

Getting the probability mass function (PMF):

	$ psql test -c "select kll_float_sketch_get_pmf(sketch, ARRAY[-2, -1, 0, 1, 2]) from kll_float_sketch_test"
	               kll_float_sketch_get_pmf
	------------------------------------------------------
	 {0.022966,0.135023,0.3383,0.343186,0.13466,0.025865}

The ARRAY[-2, -1, 0, 1, 2] of 5 split points defines 6 intervals (bins): (-inf,-2), [-2,-1), [-1,0), [0,1), [1,2), [2,inf). The result is 6 estimates of probability mass in these bins (fractions of input values that fall into the bins). These fractions can be transformed to counts (histogram) by scaling them by the factor of N (the total number of input values), which can be obtained from the sketch:

	$ psql test -c "select kll_float_sketch_get_n(sketch) from kll_float_sketch_test"
	 kll_float_sketch_get_n
	------------------------
	                1000000

In this simple example we know the value of N since we constructed this sketch, but in a general case sketches are merged across dimensions of data hypercube, so the vale of N is not known in advance.

Note that the normal distribution was used just to show the basic usage. The sketch does not make any assumptions about the distribution.

Let's create two more sketches to show merging kll_float_sketch:

	insert into kll_float_sketch_test select kll_float_sketch_build(value) from normal;
	insert into kll_float_sketch_test select kll_float_sketch_build(value) from normal;
	select kll_float_sketch_get_quantile(kll_float_sketch_merge(sketch), 0.5) from kll_float_sketch_test;
	 kll_float_sketch_get_quantile
	-------------------------------
	                    0.00332207

### Frequent strings

Consider a numeric Zipfian distribution with parameter alpha=1.1 (high skew)
and range of 2<sup>13</sup>, so that the number 1 has the highest frequency,
the number 2 appears substantially less frequently and so on.
Suppose zipf\_1p1\_8k\_100m table has 100 million random values drawn from such
a distribution, and the values are converted to strings.

Suppose the goal is to get the most frequent strings from this table. In
terms of the frequent items sketch we have to chose a threshold. Let's try
to capture values that repeat more than 1 million times, or more than 1% of
the 100 million entries in the table. According to the [error table](https://datasketches.github.io/docs/Frequency/FrequentItemsErrorTable.html),
frequent items sketch of size 2<sup>9</sup> must capture all values more
frequent then about 0.7% of the input.

The following query is to build a sketch with lg_k=9 and get results with
estimated weight above 1 million using "no false negatives" policy.
The output format is: value, estimate, lower bound, upper bound.

	$ time psql test -c "select frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch_build(9, value), 1000000) from zipf_1p1_8k_100m"
	 frequent_strings_sketch_result_no_false_negatives 
	---------------------------------------------------
	 (1,15328953,15209002,15328953)
	 (2,7156065,7036114,7156065)
	 (3,4578361,4458410,4578361)
	 (4,3334808,3214857,3334808)
	 (5,2608563,2488612,2608563)
	 (6,2135715,2015764,2135715)
	 (7,1801961,1682010,1801961)
	 (8,1557433,1437482,1557433)
	 (9,1368446,1248495,1368446)
	 (10,1216532,1096581,1216532)
	 (11,1098304,978353,1098304)
	(11 rows)
	
	real	0m38.178s

Here is an equivalent exact computation:

	$ time psql test -c "select value, weight from (select value, count(*) as weight from zipf_1p1_8k_100m group by value) t where weight > 1000000 order by weight desc"
	 value |  weight  
	-------+----------
	 1     | 15328953
	 2     |  7156065
	 3     |  4578361
	 4     |  3334808
	 5     |  2608563
	 6     |  2135715
	 7     |  1801961
	 8     |  1557433
	 9     |  1368446
	 10    |  1216532
	 11    |  1098304
	(11 rows)
	
	real	0m18.362s

In this particular case the exact computation happens to be faster. This is
just to show the basic usage. Most importantly, the sketch can be used as an "additive" metric in a data cube, and can be easily merged across dimensions.

Merging frequent_strings_sketch:

	create table frequent_strings_sketch_test(sketch frequent_strings_sketch);
	insert into frequent_strings_sketch_test select frequent_strings_sketch_build(9, value) from zipf_1p1_8k_100m;
	insert into frequent_strings_sketch_test select frequent_strings_sketch_build(9, value) from zipf_1p1_8k_100m;
	insert into frequent_strings_sketch_test select frequent_strings_sketch_build(9, value) from zipf_1p1_8k_100m;
	select frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch_merge(9, sketch), 3000000) from frequent_strings_sketch_test;
	 frequent_strings_sketch_result_no_false_negatives
	---------------------------------------------------
	 (1,45986859,45627006,45986859)
	 (2,21468195,21108342,21468195)
	 (3,13735083,13375230,13735083)
	 (4,10004424,9644571,10004424)
	 (5,7825689,7465836,7825689)
	 (6,6407145,6047292,6407145)
	 (7,5405883,5046030,5405883)
	 (8,4672299,4312446,4672299)
	 (9,4105338,3745485,4105338)
	 (10,3649596,3289743,3649596)
	 (11,3294912,2935059,3294912)
	(11 rows)

----

Disclaimer: Apache DataSketches is an effort undergoing incubation at The Apache Software Foundation (ASF), sponsored by the Apache Incubator. Incubation is required of all newly accepted projects until a further review indicates that the infrastructure, communications, and decision making process have stabilized in a manner consistent with other successful ASF projects. While incubation status is not necessarily a reflection of the completeness or stability of the code, it does indicate that the project has yet to be fully endorsed by the ASF.
