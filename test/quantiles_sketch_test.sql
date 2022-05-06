drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists quantiles_sketch_test;
create table quantiles_sketch_test(sketch quantiles_double_sketch);

-- default k
insert into quantiles_sketch_test
  select quantiles_double_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- k = 32
insert into quantiles_sketch_test
  select quantiles_double_sketch_build(value, 32)
  from (values (6), (7), (8), (9), (10)) as t(value)
;

-- get min and max values
select quantiles_double_sketch_get_quantiles(sketch, array[0, 1]) as min_max from quantiles_sketch_test;
select quantiles_double_sketch_to_string(sketch) from quantiles_sketch_test;

-- default k, median
select quantiles_double_sketch_get_quantile(quantiles_double_sketch_merge(sketch), 0.5) as median from quantiles_sketch_test;
-- k = 32, rank of value 6
select quantiles_double_sketch_get_rank(quantiles_double_sketch_merge(sketch, 32), 6) as rank from quantiles_sketch_test;

select quantiles_double_sketch_get_pmf(quantiles_double_sketch_merge(sketch, 32), array[2, 5, 7]) as pmf from quantiles_sketch_test;
select quantiles_double_sketch_get_cdf(quantiles_double_sketch_merge(sketch, 32), array[2, 5, 7]) as cdf from quantiles_sketch_test;
select quantiles_double_sketch_get_histogram(quantiles_double_sketch_merge(sketch, 32), 5) as histogram from quantiles_sketch_test;

drop table quantiles_sketch_test;
drop extension datasketches;
