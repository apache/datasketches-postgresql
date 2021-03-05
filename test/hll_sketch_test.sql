drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists hll_sketch_test;
create table hll_sketch_test(sketch hll_sketch);

-- default lgk and type
insert into hll_sketch_test
  select hll_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- lgk = 8 and type = HLL_6
insert into hll_sketch_test
  select hll_sketch_build(value, 8, 6)
  from (values (4), (5), (6), (7), (8)) as t(value)
;

select hll_sketch_get_estimate(sketch) from hll_sketch_test;
--select hll_sketch_to_string(sketch) from hll_sketch_test;

-- default lgk and type
select hll_sketch_get_estimate(hll_sketch_union(sketch)) from hll_sketch_test;
-- lgk = 8 and type = HLL_6
select hll_sketch_get_estimate(hll_sketch_union(sketch, 8, 6)) from hll_sketch_test;

drop table hll_sketch_test;
drop extension datasketches;
