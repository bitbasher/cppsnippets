color4 = [1,1,1,1]; // [-1.1:.1:5.1]
setUndef = false;
noParent = true;

parentCol= setUndef ? undef : color4;

ce=[0,0,0,1];

color([ce.r])
  cube();
translate([1.5,0,0])
  color([ce.r,ce.g])
    cube();
translate([3,0,0])
  color([ce.r,ce.g,ce.b])
    cube();
translate([4.5,0,0])
  color([ce.r,ce.g,ce.b,ce.a])
    cube();

translate([0,-1.5,0]){
  d=[.7,.7,.7,.7];

    color([d.r])
      cube();
  translate([1.5,0,0])
    color([d.r,d.g])
      cube();
  translate([3,0,0])
    color([d.r,d.g,d.b])
      cube();
  translate([4.5,0,0])
    color([d.r,d.g,d.b,d.a])
      cube();
  }