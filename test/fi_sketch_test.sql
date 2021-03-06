drop extension if exists datasketches cascade;
create extension datasketches;

drop table if exists frequent_strings_sketch_test;
create table frequent_strings_sketch_test(sketch frequent_strings_sketch);

insert into frequent_strings_sketch_test
  select frequent_strings_sketch_build(8, str)
  from (values ('a'), ('b'), ('a'), ('a'), ('c')) as t(str)
;

insert into frequent_strings_sketch_test
  select frequent_strings_sketch_build(8, str)
  from (values ('a'), ('c'), ('c'), ('b'), ('a')) as t(str)
;

select frequent_strings_sketch_result_no_false_negatives(frequent_strings_sketch_merge(8, sketch)) as frequent_strings from frequent_strings_sketch_test;
select frequent_strings_sketch_to_string(sketch) from frequent_strings_sketch_test;

drop table frequent_strings_sketch_test;
drop extension datasketches;
