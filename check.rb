p (1..50).map{|i|
  i*Math.sqrt(20000/(i*i)).ceil
}.max
