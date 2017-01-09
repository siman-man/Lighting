import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.lang.Math.*;
import java.security.*;
import java.util.*;
import java.util.regex.*;
import javax.imageio.*;

// the lights can only be placed in lattice points 0.00, 0.01, ..., 0.99, 1.00
// illumination is checked in points between lattice points 0.005, 0.015, ...
class P {
    public static final double eps = 1E-6;
    public static final int f = 100;// the number of parts we divide each unit of length into
    public long x, y;
    public double xd, yd;
    public P() {};
    public P(int x1, int y1) {
        x = x1;
        y = y1;
        xd = x / 2.0 / f;
        yd = y / 2.0 / f;
    }
    private long P2(long a) {
        return a * a;
    }
    public long dist2(P other) {
        return P2(x - other.x) + P2(y - other.y);
    }
    // to define whether the point is within lighting radius of the light
    public boolean near(P other, long d) {
        return dist2(other) <= P2(2 * d * f);
    }
    public String toString() {
        return "(" + xd + "," + yd + ")";
    }
}

class Wall {
    public P start, end;
    public Wall(P st, P e) {
        start = st;
        end = e;
    }
    public String toString() {
        return "[" + start + " - " + end + "]";
    }
    // check whether a-b segment intersects c-d segment (1-dimension)
    public static boolean boundBoxIntersect(long a, long b, long c, long d) {
        return Math.max(Math.min(a, b), Math.min(c, d)) <= 
               Math.min(Math.max(a, b), Math.max(c, d));
    }
    // calculate oriented area of ABC triangle
    private static long orientedAreaSign(P a, P b, P c) {
        long area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        return area == 0 ? 0 : area / Math.abs(area);
    }
    public boolean intersect(Wall other) {
        // two segments intersect if 1) their bounding boxes intersect and 2) oriented areas of triangles have different signs
        return boundBoxIntersect(start.x, end.x, other.start.x, other.end.x) &&
               boundBoxIntersect(start.y, end.y, other.start.y, other.end.y) &&
               orientedAreaSign(start, end, other.start) * orientedAreaSign(start, end, other.end) <= 0 && 
               orientedAreaSign(other.start, other.end, start) * orientedAreaSign(other.start, other.end, end) <= 0;
    }
}

public class LightingVis {
    static int maxSize = 50, minSize = 10;
    static int maxLights = 20, minLights = 2;
    static int maxD = 10, minD = 2;

    int S;                  // size of the room to light
    char[][] map;           // map of the room: . for empty space, # for a wall
    int D;                  // distance at which light from each source stops working
    int maxL;               // the maximum number of lights you can place

    int L;                  // number of lights actually placed
    P[] lights;             // lights placed
    ArrayList<Wall> walls;  // walls extracted from the map
    
    String errmes;
    int[][] points;
    // -----------------------------------------
    void extractWalls() {
        walls = new ArrayList<>();
        // we don't need outer walls (walls between map and outside of the map), since all light it within the map
        // a wall is a vertical or horizontal line which on each 1-length piece is bordered by both wall and empty space
        // horizontal walls
        for (int i = 0; i < S - 1; ++i) {
            int j = 0;
            while (j < S) {
                // check whether j is start of the wall
                if (map[i][j] == map[i+1][j]) {
                    j++;
                    continue;
                }
                // if it is, keep increasing it until the wall ends
                P start = new P(j * 2 * P.f, (i+1) * 2 * P.f);
                while (j < S && map[i][j] != map[i+1][j]) {
                    j++;
                }
                P end = new P(j * 2 * P.f, (i+1) * 2 * P.f);
                Wall w = new Wall(start, end);
                walls.add(w);
                if (debug) {
                    addFatalError("Horizontal wall " + w);
                }
            }
        }

        // vertical walls
        for (int j = 0; j < S - 1; ++j) {
            int i = 0;
            while (i < S) {
                // check whether j is start of the wall
                if (map[i][j] == map[i][j+1]) {
                    i++;
                    continue;
                }
                // if it is, keep increasing it until the wall ends
                P start = new P((j+1) * 2 * P.f, i * 2 * P.f);
                while (i < S && map[i][j] != map[i][j+1]) {
                    i++;
                }
                P end = new P((j+1) * 2 * P.f, i * 2 * P.f);
                Wall w = new Wall(start, end);
                walls.add(w);
                if (debug) {
                    addFatalError("Vertical wall " + w);
                }
            }
        }
    }
    // -----------------------------------------
    void generate(String seedStr) {
    try {
        // generate test case
        SecureRandom r1 = SecureRandom.getInstance("SHA1PRNG");
        long seed = Long.parseLong(seedStr);
        r1.setSeed(seed);

        S = r1.nextInt(maxSize - minSize + 1) + minSize;
        D = r1.nextInt(maxD - minD + 1) + minD;
        maxL = r1.nextInt(maxLights - minLights + 1) + minLights;
        if (seed == 1) {
            S = 5;
            D = 3;
            maxL = 2;
        } else if (seed == 2) {
            S = maxSize;
            D = maxD;
            maxL = maxLights;
        }

        // generate the map of the room
        double prob = r1.nextDouble() * 0.2 + 0.1;

        map = new char[S][S];
        for (int i = 0; i < S; ++i)
        for (int j = 0; j < S; ++j) {
            map[i][j] = (r1.nextDouble() < prob ? '#' : '.');
        }

        if (debug) {
            System.out.println("Size of the room S = " + S);
            System.out.println("Room map: ");
            for (int i = 0; i < S; ++i) {
                System.out.println(new String(map[i]));
            }
            System.out.println("Light distance D = " + D);
            System.out.println("Max number of lights maxL = " + maxL);
        }

        extractWalls();
        
        if (vis) {
            col = new int[maxL];
            for (int i = 0; i < maxL; ++i)
                col[i] = r1.nextInt(0xBBBBBB) + 0x444444;
        }
    }
    catch (Exception e) {
        System.err.println("An exception occurred while generating test case.");
        e.printStackTrace();
    }
    }
    // -----------------------------------------
    private static String coordRegexp = "[0-9]+\\.[0-9][0-9]";
    int getCoord(String c) {
        errmes = "";
        if (!c.matches(coordRegexp)) {
            errmes = "Coordinate [" + c + "] of your return doesn't match required format [" + coordRegexp + "].";
            return -1;
        }
        String[] p = c.split("\\.");
        int p0, p1;
        try {
            p0 = Integer.parseInt(p[0]);
        } catch (Exception e) {
            errmes = "Exception while trying to parse integer part of coordinate [" + c + "] [" + p[0] + "]: " + e.getMessage();
            return -1;
        }
        try {
            p1 = Integer.parseInt(p[1]);
        } catch (Exception e) {
            errmes = "Exception while trying to parse fractional part of coordinate [" + c + "] [" + p[1] + "]: " + e.getMessage();
            return -1;
        }
        if (p0 < 0 || p0 > S || p0 == S && p1 > 0) {
            errmes = "Coordinate [" + c + "] is outside of the map.";
            return -1;
        }
        return (p0 * P.f + p1) * 2;
    }
    // -----------------------------------------
    void markPointsIlluminated(int lightInd) {
        P light = lights[lightInd];
        // first, find bounding box of the light and only get the walls which intersect it
        long boxX1 = Math.max(0, light.x - 2 * P.f * D);
        long boxX2 = Math.min(2 * (P.f * S - 1), light.x + 2 * P.f * D);
        long boxY1 = Math.max(0, light.y - 2 * P.f * D);
        long boxY2 = Math.min(2 * (P.f * S - 1), light.y + 2 * P.f * D);
        
        ArrayList<Integer> localWallsInd = new ArrayList<>();
        for (int i = 0; i < walls.size(); ++i) {
            Wall w = walls.get(i);
            if (Wall.boundBoxIntersect(boxX1, boxX2, w.start.x, w.end.x) && 
                Wall.boundBoxIntersect(boxY1, boxY2, w.start.y, w.end.y)) {
                localWallsInd.add(i);
            }
        }
        
        // now iterate throw points in the bounding box and see whether they are illuminated
        // using only local walls, not the whole list
        for (int x = (int)boxX1 / 2; x <= boxX2 / 2; ++x)
        for (int y = (int)boxY1 / 2; y <= boxY2 / 2; ++y) {
            if (points[y][x] != 0) {
                // point is either already illuminated or part of the wall
                continue;
            }
            P point = new P(x * 2 + 1, y * 2 + 1);
            if (!light.near(point, D))
                continue;
            boolean ok = true;
            Wall beam = new Wall(point, light);
            for (Integer ind : localWallsInd) {
                if (beam.intersect(walls.get(ind.intValue()))) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                points[y][x] = 1 + lightInd;
        }
    }
    // -----------------------------------------
    public double runTest(String seed) {
    try {
        generate(seed);

        String[] mapArg = new String[S];
        for (int i = 0; i < S; ++i)
            mapArg[i] = new String(map[i]);

        String[] ret = setLights(mapArg, D, maxL);

        // parse return value and convert it to a list of light locations
        if (ret == null) {
            addFatalError("Failed to get result from setLights.");
            return 0.0;
        }
        L = ret.length;
        if (L > maxL) {
            addFatalError("You can place at most " + maxL + " lights, and you placed " + L + ".");
            return 0.0;
        }
        
        lights = new P[L];
        for (int i = 0; i < L; ++i) {
            String[] s = ret[i].split(" ");
            if (s.length != 2) {
                addFatalError("Each element of your return must be formatted as 'X Y'.");
                return 0.0;
            }
            // both x(column) and y(row) must be floating-point numbers with exactly two places after decimal point
            // which correspond to real light position
            int x = getCoord(s[0]);
            if (errmes != "") {
                addFatalError(errmes);
                return 0.0;
            }
            int y = getCoord(s[1]);
            if (errmes != "") {
                addFatalError(errmes);
                return 0.0;
            }
            lights[i] = new P(x, y);
        }

        // -1 for wall point, 0 for dark point, 1 for illuminated point
        points = new int[S * P.f][S * P.f];
        for (int i = 0; i < S * P.f; ++i)
            Arrays.fill(points[i], 0);

        // mark wall points beforehand
        for (int r = 0; r < S; ++r)
        for (int c = 0; c < S; ++c) {
            if (map[r][c] != '#')
                continue;
            for (int x = c * P.f; x < (c + 1) * P.f; ++x)
            for (int y = r * P.f; y < (r + 1) * P.f; ++y) {
                points[y][x] = -1;
            }
        }
        
        // now, for each source of light find points illuminated by it
        for (int i = 0; i < L; ++i) {
            markPointsIlluminated(i);
        }
        
        if (vis) {
            BufferedImage bi = new BufferedImage(S * P.f + 1, S * P.f + 1, BufferedImage.TYPE_INT_RGB);
            Graphics2D g2 = (Graphics2D)bi.getGraphics();
            g2.setColor(Color.BLACK);
            g2.fillRect(0, 0, S * P.f, S * P.f);

            for (int r = 0; r < S * P.f; ++r)
            for (int c = 0; c < S * P.f; ++c)
                if (points[r][c] != -1)
                    bi.setRGB(c, r, points[r][c] == 0 ? 0x777777 : col[points[r][c] - 1]);

            // draw the light fixtures on top of colors
            for (int i = 0; i < L; ++i) {
                int x = (int)lights[i].x / 2;
                int y = (int)lights[i].y / 2;
                bi.setRGB(x, y, 0);
                if (x > 0)
                    bi.setRGB(x - 1, y, 0);
                if (y > 0)
                    bi.setRGB(x, y - 1, 0);
                if (x < S * P.f - 1)
                    bi.setRGB(x + 1, y, 0);
                if (y < S * P.f - 1)
                    bi.setRGB(x, y + 1, 0);
            }

            // draw the map of points illuminated to a png file (to handle the scale issues easier)
            ImageIO.write(bi,"png", new File(seed+".png"));
        }

        // check all points within the map and count illuminated ones
        int nIllum = 0, nTotal = 0;
        for (int r = 0; r < S; ++r)
        for (int c = 0; c < S; ++c) {
            if (map[r][c] == '#')
                continue;
            for (int x = 0; x < P.f; ++x)
            for (int y = 0; y < P.f; ++y) {
                // the point we need to check is middle of square of size 1/P.f
                nTotal++;
                if (points[r * P.f + y][c * P.f + x] > 0)
                    nIllum++;
            }
        }

        return nIllum * 1.0 / nTotal;
    }
    catch (Exception e) {
        System.err.println("An exception occurred while trying to get your program's results.");
        e.printStackTrace();
        return 0;
    }
    }
// ------------- visualization part ------------
    static String exec;
    static boolean vis, debug;
    static Process proc;
    int[] col;
    InputStream is;
    OutputStream os;
    BufferedReader br;
    // -----------------------------------------
    String[] setLights(String[] map, int D, int maxL) throws IOException {
        if (proc == null) {
            return new String[0];
        }
        StringBuffer sb = new StringBuffer();
        sb.append(map.length).append("\n");
        for (String st : map) {
            sb.append(st).append("\n");
        }
        sb.append(D).append("\n");;
        sb.append(maxL).append("\n");;
        os.write(sb.toString().getBytes());
        os.flush();

        int retN = Integer.parseInt(br.readLine());
        String[] ret = new String[retN];
        for (int i = 0; i < retN; ++i)
            ret[i] = br.readLine();
        return ret;
    }
    // -----------------------------------------
    public LightingVis(String seed) {
      try {
        if (exec != null) {
            try {
                Runtime rt = Runtime.getRuntime();
                proc = rt.exec(exec);
                os = proc.getOutputStream();
                is = proc.getInputStream();
                br = new BufferedReader(new InputStreamReader(is));
                new ErrorReader(proc.getErrorStream()).start();
            } catch (Exception e) { e.printStackTrace(); }
        }
        System.out.println("Score = " + runTest(seed));
        if (proc != null)
            try { proc.destroy(); }
            catch (Exception e) { e.printStackTrace(); }
      }
      catch (Exception e) { e.printStackTrace(); }
    }
    // -----------------------------------------
    public static void main(String[] args) {
        String seed = "1";
        vis = true;
        for (int i = 0; i<args.length; i++)
        {   if (args[i].equals("-seed"))
                seed = args[++i];
            if (args[i].equals("-exec"))
                exec = args[++i];
            if (args[i].equals("-novis"))
                vis = false;
            if (args[i].equals("-debug"))
                debug = true;
        }
        LightingVis f = new LightingVis(seed);
    }
    // -----------------------------------------
    void addFatalError(String message) {
        System.out.println(message);
    }
}

class ErrorReader extends Thread{
    InputStream error;
    public ErrorReader(InputStream is) {
        error = is;
    }
    public void run() {
        try {
            byte[] ch = new byte[50000];
            int read;
            while ((read = error.read(ch)) > 0)
            {   String s = new String(ch,0,read);
                System.out.print(s);
                System.out.flush();
            }
        } catch(Exception e) { }
    }
}
