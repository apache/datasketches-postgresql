drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists aod_sketch_test;
create table aod_sketch_test(sketch aod_sketch);

-- default lgk
insert into aod_sketch_test
  select aod_sketch_build(key, aod)
  from (values (1, array[1]), (2, array[1]), (3, array[1]), (4, array[1]), (5, array[1])) as t(key, aod)
;

-- lgk = 16
insert into aod_sketch_test
  select aod_sketch_build(key, aod, 16)
  from (values (4, array[1]), (5, array[1]), (6, array[1]), (7, array[1]), (8, array[1])) as t(key, aod)
;

select aod_sketch_get_estimate(sketch) from aod_sketch_test;
select aod_sketch_to_string(sketch) from aod_sketch_test;

-- default lgk
select aod_sketch_get_estimate(aod_sketch_union(sketch)) from aod_sketch_test;
-- lgk = 16
select aod_sketch_get_estimate(aod_sketch_union(sketch, 16)) from aod_sketch_test;

select aod_sketch_get_estimate(aod_sketch_intersection(sketch)) from aod_sketch_test;

select aod_sketch_get_estimate(aod_sketch_a_not_b(aod_sketch_build(key1, aod1), aod_sketch_build(key2, aod2)))
from (values (1, array[1], 2, array[1]), (2, array[1], 3, array[1]), (3, array[1], 4, array[1])) as t(key1, aod1, key2, aod2);

select aod_sketch_to_kll_float_sketch(sketch, 1) from aod_sketch_test;
select aod_sketch_to_means(sketch) from aod_sketch_test;
select aod_sketch_to_variances(sketch) from aod_sketch_test;

select aod_sketch_students_t_test(aod_sketch_build(key1, aod1), aod_sketch_build(key2, aod2))
from (values
  (1, array[1], 1, array[1.1]),
  (2, array[0.9], 2, array[1]),
  (3, array[1.1], 3, array[1.2]),
  (4, array[1], 4, array[1.1])
) as t(key1, aod1, key2, aod2);

drop table aod_sketch_test;
drop extension datasketches;
