drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists theta_sketch_test;
create table theta_sketch_test(sketch theta_sketch);

-- default lgk
insert into theta_sketch_test
  select theta_sketch_build(value)
  from (values (1), (2), (3), (4), (5)) as t(value)
;

-- lgk = 16
insert into theta_sketch_test
  select theta_sketch_build(value, 16)
  from (values (4), (5), (6), (7), (8)) as t(value)
;

select theta_sketch_get_estimate(sketch) from theta_sketch_test;
--select theta_sketch_to_string(sketch) from theta_sketch_test;

-- default lgk
select theta_sketch_get_estimate(theta_sketch_union(sketch)) from theta_sketch_test;
-- lgk = 16
select theta_sketch_get_estimate(theta_sketch_union(sketch, 16)) from theta_sketch_test;

select theta_sketch_get_estimate(theta_sketch_intersection(sketch)) from theta_sketch_test;

select theta_sketch_get_estimate(theta_sketch_a_not_b(theta_sketch_build(value1), theta_sketch_build(value2)))
from (values (1, 2), (2, 3), (3, 4)) as t(value1, value2);

drop table theta_sketch_test;
drop extension datasketches;
