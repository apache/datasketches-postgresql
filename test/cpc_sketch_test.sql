drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists cpc_sketch_test;
create table cpc_sketch_test(sketch cpc_sketch);

-- default lgk
insert into cpc_sketch_test
  select cpc_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- lgk = 8
insert into cpc_sketch_test
  select cpc_sketch_build(value, 8)
  from (values (4), (5), (6), (7), (8)) as t(value)
;

select cpc_sketch_get_estimate(sketch) from cpc_sketch_test;
--select cpc_sketch_to_string(sketch) from cpc_sketch_test;

-- default lgk and type
select cpc_sketch_get_estimate(cpc_sketch_union(sketch)) from cpc_sketch_test;
-- lgk = 8
select cpc_sketch_get_estimate(cpc_sketch_union(sketch, 8)) from cpc_sketch_test;

drop table cpc_sketch_test;
drop extension datasketches;
