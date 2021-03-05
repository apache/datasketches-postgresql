drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists req_sketch_test;
create table req_sketch_test(sketch req_float_sketch);

-- default k
insert into req_sketch_test
  select req_float_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- k = 20
insert into req_sketch_test
  select req_float_sketch_build(value, 20)
  from (values (6), (7), (8), (9), (10)) as t(value)
;

-- get min and max values
select req_float_sketch_get_quantiles(sketch, array[0, 1]) as min_max from req_sketch_test;
select req_float_sketch_to_string(sketch) from req_sketch_test;

-- default k, median
select req_float_sketch_get_quantile(req_float_sketch_merge(sketch), 0.5) as median from req_sketch_test;
-- k = 20, rank of value 6
select req_float_sketch_get_rank(req_float_sketch_merge(sketch, 20), 6) as rank from req_sketch_test;

drop table req_sketch_test;
drop extension datasketches;
