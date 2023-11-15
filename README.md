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
# PostgreSQL DataSketches Extension

Please visit our [DataSketches website](https://datasketches.apache.org) for more information. 

If you are interested in making contributions to this site please see our 
[Community](https://datasketches.apache.org/docs/Community/) page for how to contact us.

---

## This extension currently supports the following sketches:

- CPC (Compressed Probabilistic Counting) sketch - very compact (smaller than HLL when serialized) distinct-counting sketch
- HLL sketch - very compact distinct-counting sketch based on HyperLogLog algorithm
- Theta sketch - distinct counting with set operations (union, intersection, a-not-b)
- Array Of Doubles (AOD) sketch - a kind of Tuple sketch with array of double values associated with each key
- KLL (float and double) quantiles sketch - for estimating distributions: quantile, rank, PMF (histogram), CDF
- Quantiles sketch (inferior to KLL, for long-term support of data sets)
- Frequent strings sketch - capture the heaviest items (strings) by count or by some other weight

## Building this PostgreSQL Extension Requires 4 Dependencies

* C++11
* Boost version 1.75.0 we know this works, older and newer may work as well
* PostgreSQL database versions 9.6 and higher
* [DataSketches *C++ Core*](https://github.com/apache/datasketches-cpp) version 5.0.0 or later

## Preparing the Components

### Installing PostgreSQL Database

* If you think your system already has the PostgreSQL database installed, try running '*pg_config*' to check the version.
This will also indicate if the path to the PostgreSQL executables have been set up. If it is already installed, you can
move to the next step.
* If it is not installed, here are the installation instructions:
[PostgreSQL documentation](https://www.postgresql.org/docs/current/tutorial-start.html)

### Preparing the PostgreSQL DataSketches Extension
There are two different ways to obtain this extension.

* A single download from the **PGXN distribution**, which already contains the DataSketches **C++ Core** component,
* Or download two separate packages, DataSketches **C++ Core** and DataSketches **C++ PostgreSQL Extension**, 
both of which can be obtained from
[DataSketches download](https://datasketches.apache.org/docs/Community/Downloads.html).

#### Downloading From the PGXN distribution
    
* Download the PostgreSQL **datasketches extension** from [PGXN](https://pgxn.org/dist/datasketches/)
* Unzip the package (the DataSketches C++ Core is included)
* Proceed to [Preparing Boost](#toc_10)

#### Downloading separate packages

##### Downloading DataSketches *C++ PostgreSQL Extension*

* Download the DataSketches **C++ PostgreSQL Extension** into its own directory from
[DataSketches download](https://datasketches.apache.org/docs/Community/Downloads.html)
* Or from [GitHub](https://github.com/apache/datasketches-postgresql) selecting the correct version release tag.
* Unzip the package

##### Downloading DataSketches C++ Core

* Download the DataSketches **C++ Core** into its own directory from
[DataSketches download](https://datasketches.apache.org/docs/Community/Downloads.html) selecting the correct version.
(You may need to check the archives: "Recent ZIP Releases".)
* Or from [GitHub](https://github.com/apache/datasketches-cpp) selecting the correct version release tag.
* Unzip and create a link in the same directory where the *DataSketches C++ PostgreSQL Extension* is:
    * *datasketches-cpp* -> apache-datasketches-cpp-\<version\>

### Preparing Boost

* [Download Boost](https://www.boost.org/users/download/)
* Unzip and create a link in the same directory where the *DataSketches C++ PostgreSQL Extension* is:
    * *boost* -> boost_\<version\>

### Building

From the command line at the PGXN installed root or the root of the C++ PostgreSQL Adaptor run:

* *make*
* *sudo make install*  #sudo may not be required depending on how PostgreSQL was installed

NOTE: On MacOSX Mojave, if you see a warning similar to this:

```
clang: warning: no such sysroot directory: 
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk’
[-Wmissing-sysroot]`
```
and the compilation fails because of not being able to find system include files, this is a known OSX problem. 
There are known solutions on the Internet.

### Verifying installation with a test database

* Make sure that PostgreSQL is running. For instance, using Homebrew on MacOSX, start the service:
    * *brew services start postgresql*
* Create a test database if it does not exist yet:
    * *createdb test*
* Run the client (console) using the test database, create the extension and try some of the datasketches functions:
    * *psql test*
    * test=# *create extension datasketches;*
    * test=# *select cpc\_sketch\_to\_string(cpc\_sketch\_build(1));*

The select statement above should produce something similar to the following:

```
       cpc_sketch_to_string        
-----------------------------------
 ### CPC sketch summary:          +
    lg_k           : 11           +
    seed hash      : 93cc         +
    C              : 1            +
    flavor         : 1            +
    merged         : false        +
    HIP estimate   : 1            +
    kxp            : 2047.5       +
    interesting col: 0            +
    table entries  : 1            +
    window         : not allocated+
 ### End sketch summary           +
 
(1 row)
```
### Quitting and Stopping the PostgreSQL database
In the same console that you started the database and ran the test

To quit the test:

* test=# *\q*

To stop the database server:

* *brew services stop postgresql* (this is platform dependent)


## Docker

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

### Estimating quantiles, ranks and histograms with KLL sketch

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

In this simple example we know the value of N since we constructed this sketch, but in a general case sketches are merged across dimensions of data hypercube, so the value of N is not known in advance.

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
terms of the frequent items sketch, we have to choose a threshold. Let's try
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
