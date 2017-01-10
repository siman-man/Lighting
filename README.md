## 問題文

Your friend an interior designer asked you to help him design a lighting scheme for a series of apartments he's working on.
あなたの友人はインテリアデザイナであなたは彼に部屋のライトニングについて相談を行いました。

The apartment map is a square of S x S cells. Each cell is either empty or occupied by a wall.
アパートはS x Sの大きさの正方形のセルで構成されており、各セルは壁か空に分けることが出来ます。

You have to place several light fixtures in the apartment. All fixtures are identical, each one casts light from a single-point source
あなたはアパートに幾つかの固定ライトを設置する必要があります。 全ての固定スタンドは独立しており、単一光源として機能します
and can illuminate an area of the apartment shaped as a circle. The walls block the light, and the inside of the walls can't be illuminated.
また円形状に周囲のマスを照らすことが出来ます。ブロックが存在する場合、それ以降のフィールドは照らされません
If a light fixture is placed on the wall (i.e. on the border between empty cell and wall cell), all its light is blocked by this wall.
もし、壁にライトを設置した場合には、全ての光がその壁によって遮られます。

Your goal is to place light fixtures so that the area of empty cells illuminated by them is maximized.
あなたの目的は空のセルをライトアップ出来る最大の設置パターンを見つけることです。


## 実装

Your code must implement one method setLights(String[] map, int D, int L):
あなたは `setLights` メソッドを実装する必要があります。


* map is the apartment map. map[i][j] describes the square in row i and column j: '.' denotes empty space, and '#' denotes a column.
mapはアパートの見取り図です。map[i][j]はi行目のj番目の列の状態を表します。 '.' は何もない空間を表し '#' は壁を表します。

* D is the distance from the light fixture to the furthest point which can be illuminated by it.
D はスタンドの光源が届く距離を表します。

* L is the maximum number of light fixtures that can be put in the apartment.
L アパートに設置可能なライトの最大数を表します。

The return from this method will describe the points at which light fixtures should be placed. Each element of the return is a String which
返り値にはライトの設置箇所を指定します。
describes one point and is formatted as "X Y". X and Y are floating-point coordinates of the point (X corresponds to columns dimension, and Y to rows)
"X Y" のフォーマットで指定してください。
written in fixed-point notation with exactly two digits after decimal point.


## 制限

* The size of the map S will be between 10 and 50, inclusive.
マップのサイズは10 - 50の間です

* The number of light fixtures allowed L will be between 2 and 20, inclusive.
ライトの数は2 - 20の間です

* The value of D will be between 2 and 10, inclusive.
ライトの光源の強さは2 - 10です
