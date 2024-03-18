drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists kll_sketch_test;
create table kll_sketch_test(sketch kll_float_sketch);

-- default k
insert into kll_sketch_test
  select kll_float_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- k = 20
insert into kll_sketch_test
  select kll_float_sketch_build(value, 20)
  from (values (6), (7), (8), (9), (10)) as t(value)
;

-- get min and max values
select kll_float_sketch_get_min_item(sketch) as min_item from kll_sketch_test;
select kll_float_sketch_get_max_item(sketch) as max_item from kll_sketch_test;
select kll_float_sketch_get_quantiles(sketch, array[0, 1]) as min_max from kll_sketch_test;
select kll_float_sketch_to_string(sketch) from kll_sketch_test;

-- default k, median
select kll_float_sketch_get_quantile(kll_float_sketch_merge(sketch), 0.5) as median from kll_sketch_test;
-- k = 20, rank of value 6
select kll_float_sketch_get_rank(kll_float_sketch_merge(sketch, 20), 6) as rank from kll_sketch_test;

select kll_float_sketch_get_pmf(kll_float_sketch_merge(sketch, 20), array[2, 5, 7]) as pmf from kll_sketch_test;
select kll_float_sketch_get_cdf(kll_float_sketch_merge(sketch, 20), array[2, 5, 7]) as cdf from kll_sketch_test;
select kll_float_sketch_get_histogram(kll_float_sketch_merge(sketch, 20), 5) as histogram from kll_sketch_test;

drop table kll_sketch_test;
drop extension datasketches;
