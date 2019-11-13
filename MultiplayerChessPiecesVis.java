import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.security.*;
import java.util.*;
import java.net.URL;
import javax.swing.*;
import javax.imageio.*;


public class MultiplayerChessPiecesVis {

    //test parameters
    private static int minN = 8, maxN = 50;
    private static int minC = 2, maxC = 8;
    private static double minWallP = 0.15, maxWallP = 0.65;       
    
    //constants
    final static int[] KingDr={1,1, 1,0, 0,-1,-1,-1};   //used for queen too
    final static int[] KingDc={1,0,-1,1,-1, 1, 0,-1};  
    final static int[] RookDr={ 0,1,0,-1};
    final static int[] RookDc={-1,0,1, 0};
    final static int[] BishopDr={1, 1,-1,-1};
    final static int[] BishopDc={1,-1, 1,-1};    
		final static int[] KnightDr={2, 2,-2,-2,1, 1,-1,-1};
		final static int[] KnightDc={1,-1, 1,-1,2,-2, 2,-2};
    final static char EMPTY='.';
    final static char WALL='#';
    //notation from https://en.wikipedia.org/wiki/Descriptive_notation
    final static char KING='K';
    final static char QUEEN='Q';
    final static char ROOK='R';
    final static char BISHOP='B';
    final static char KNIGHT='N';
    final static char[] names={KING,QUEEN,ROOK,BISHOP,KNIGHT};
    final static String[] names2={"King","Queen","Rook","Bishop","Knight"};
    
    

    //inputs
    int N;    
    int C;    
    char[][] Grid;
    int[] Points;
    
    //outputs
    int Score;  
    int[] Scores;
    
    // -----------------------------------------
    void generate(String seedStr) {
      try {
        // generate test case
        SecureRandom r1 = SecureRandom.getInstance("SHA1PRNG"); 
        long seed = Long.parseLong(seedStr);
        r1.setSeed(seed);
        
        //generate grid size
        N = r1.nextInt(maxN - minN + 1) + minN;
        
        if (seed == 1) N = minN;
        if (seed == 2) N = maxN;                
        
        //generate number of colours
        C = r1.nextInt(maxC - minC + 1) + minC;      
        
        //generate wall probability
        double wallP = r1.nextDouble()*(maxWallP - minWallP) + minWallP;

        
        if (seed == 1) wallP = 0.19962386685404293;
       

        //generate grid with walls
        Grid=new char[N][N];
        for (int i=0; i<N; i++)
          for (int k=0; k<N; k++)
            if (r1.nextDouble() < wallP)
              Grid[i][k]=WALL;
            else
              Grid[i][k]=EMPTY;

        //generate points for each piece
        Points=new int[names.length];
        for (int i=0; i<names.length; i++)
        {
          if (names[i]==KING)
            Points[i]=r1.nextInt(8 - 3 + 1) + 3;
          if (names[i]==QUEEN)
            Points[i]=r1.nextInt(4*N - 3*N + 1) + 3*N;            
          if (names[i]==ROOK)
            Points[i]=r1.nextInt(N*5/2 - N*3/2 + 1) + N*3/2;               
          if (names[i]==BISHOP)
            Points[i]=r1.nextInt(2*N - N + 1) + N;               
          if (names[i]==KNIGHT)
            Points[i]=r1.nextInt(8 - 2 + 1) + 2;                           
        }
                   
        if (Debug) printTest(seedStr);        
      }
      catch (Exception e) {
        addFatalError("An exception occurred while generating test case.");
        e.printStackTrace(); 
      }
    }
    
    void printTest(String seed)
    {
        System.out.println("seed = "+seed);
        System.out.println("N = "+N);
        System.out.println("C = "+C);
        System.out.print("Points:");
        for (int i : Points) System.out.print(" "+i);
        System.out.println();
        System.out.println("Grid:");
        for (int i=0; i<N; i++)
        {
          for (int k=0; k<N; k++) System.out.print(Grid[i][k]);
          System.out.println();
        }
    }
            
        
    // -----------------------------------------
    public double runTest(String seed) {
      try {
        generate(seed);
        
        char[] solution;

        if (proc != null) {
            // call the solution
            try { 
                solution = callSolution();
            } catch (Exception e) {
                addFatalError("Failed to get result from your solution.");
                return -1.0;
            }

            // check the return and score it
            if (solution == null) {
                addFatalError("Your return contained an invalid number of elements.");
                return -1.0;
            }
            if (solution.length != 2*N*N) {
                addFatalError("Your return did not contain " + (2*N*N) + " elements.");
                return -1.0;
            }
            
            
            //construct output board and check for invalid characters
            Board=new char[N][N];
            for (int i=0,cur=0; i<N; i++)
              for (int k=0; k<N; k++,cur++)
              {
                Board[i][k]=solution[cur];
                
                if (!isLegal(Board[i][k]))
                {
                  addFatalError("Cell ("+i+","+k+") has illegal character "+Board[i][k]);
                  return -1.0;                
                }
                if (Grid[i][k]==WALL && Board[i][k]!=WALL)
                {
                  addFatalError("You cannot remove the wall at cell ("+i+","+k+")");
                  return -1.0;                                
                }
                if (Grid[i][k]==EMPTY && Board[i][k]==WALL)
                {
                  addFatalError("You cannot add a wall to cell ("+i+","+k+")");
                  return -1.0;                                
                }                  
              }
              
            
            Players=new int[N][N];
            for (int i=0,cur=N*N; i<N; i++)
              for (int k=0; k<N; k++,cur++)
              {
                if (Board[i][k]==WALL || Board[i][k]==EMPTY) continue;
                
                if (solution[cur]<'0' || solution[cur]>='0'+C)
                {
                  addFatalError("Illegal colour "+solution[cur]+" at cell ("+i+","+k+")");
                  return -1.0;
                }
                
                Players[i][k]=(int)(solution[cur]-'0');             
              }            
                                    
              
              
            //check that no piece attacks any other piece of a different colour
            for (int r=0; r<N; r++)
              for (int c=0; c<N; c++)
              {
                if (Board[r][c]==KING && checkAttackSimple(r,c,KingDr,KingDc,"King"))
                  return -1.0;
                if (Board[r][c]==QUEEN && checkAttackComplex(r,c,KingDr,KingDc,"Queen"))
                  return -1.0;
                if (Board[r][c]==ROOK && checkAttackComplex(r,c,RookDr,RookDc,"Rook"))
                  return -1.0;                         
                if (Board[r][c]==BISHOP && checkAttackComplex(r,c,BishopDr,BishopDc,"Bishop"))
                  return -1.0;                                                                        
                if (Board[r][c]==KNIGHT && checkAttackSimple(r,c,KnightDr,KnightDc,"Knight"))
                  return -1.0;                                                      
              }


            if (Debug)
            {
              System.err.println("received board pieces");
              for (int r=0; r<N; r++)
              {
                for (int c=0; c<N; c++) System.err.print(Board[r][c]);
                System.err.println();
              }
              System.err.println("received board player id");
              for (int i=0,cur=N*N; i<N; i++)
              {
                for (int k=0; k<N; k++,cur++)
                  System.err.print(solution[cur]);                  
                System.err.println();
              }              
            }
              
              
            //compute score
            Scores=new int[C];
            
            for (int r=0; r<N; r++)
              for (int c=0; c<N; c++)
                for (int m=0; m<names.length; m++)
                  if (names[m]==Board[r][c])
                    Scores[Players[r][c]]+=Points[m];              
                    
                    
            Score=Integer.MAX_VALUE;                                    
            for (int i=0; i<C; i++) Score=Math.min(Score,Scores[i]);
              
            
            if (vis) {
                jf.setSize(N*SZ+230,N*SZ+50);
                jf.setVisible(true);
                draw();
            }   
        }
       
        return Score;
      }
      catch (Exception e) { 
        addFatalError("An exception occurred while trying to get your program's results.");
        e.printStackTrace(); 
        return -1.0;
      }
    }
    
    
    boolean checkAttackSimple(int r, int c, int[] dr, int[] dc, String piece)
    {
      for (int m=0; m<dr.length; m++)
      {
        int r2=r+dr[m];
        int c2=c+dc[m];
        if (inGrid(r2,c2) && isPiece(Board[r2][c2]) && Players[r][c]!=Players[r2][c2])
        {
          addFatalError(piece+" at ("+r+","+c+") attacks piece at ("+r2+","+c2+")");
          return true;                                                      
        }
      }      
      return false;
    }

    //for sliding pieces
    boolean checkAttackComplex(int r, int c, int[] dr, int[] dc, String piece)
    {   
      for (int m=0; m<dr.length; m++)
      {
        int r2=r;
        int c2=c;
        while(inGrid(r2,c2) && Board[r2][c2]!=WALL)
        {
          r2+=dr[m];
          c2+=dc[m];
          if (inGrid(r2,c2) && isPiece(Board[r2][c2]))
          {
            //blocked by piece of own colour
            if (Players[r][c]==Players[r2][c2]) break;
            //attacking a piece of different colour is not allowed
            else
            {
              addFatalError(piece+" at ("+r+","+c+") attacks piece at ("+r2+","+c2+")");
              return true;                                                      
            }
          }      
        }
      }
      return false;
    }    
    
    boolean isLegal(char c)
    {
      return c==EMPTY || c==WALL || c==KING || c==QUEEN || c==ROOK || c==BISHOP || c==KNIGHT;
    }
    
    boolean isPiece(char c)
    {
      return c==KING || c==QUEEN || c==ROOK || c==BISHOP || c==KNIGHT;
    }    
    
    boolean inGrid(int r, int c)
    {
      return (r>=0 && r<N && c>=0 && c<N);
    }
    
    
// ------------- visualization part ------------
    JFrame jf;
    Vis v;
    static String exec;
    static boolean vis;
    static Process proc;
    InputStream is;
    OutputStream os;
    BufferedReader br;
    static int SZ;
    static boolean Debug;
    static char[][] Board;
    static int[][] Players;
    //pics from https://commons.wikimedia.org/wiki/Category:PNG_chess_pieces/Standard_transparent
    Image[] pics;   
    
    
    
    // -----------------------------------------
    private char[] callSolution() throws IOException, NumberFormatException {
        if (exec == null) return null;

        String s=N+"\n";
        os.write(s.getBytes());
        s=C+"\n";
        os.write(s.getBytes());
        for (int i=0; i<N; i++)
          for (int k=0; k<N; k++)
          {
            s=Grid[i][k]+"\n";
            os.write(s.getBytes());
          }          
          
        for (int i : Points)
        {
            s=i+"\n";
            os.write(s.getBytes());
        }        
        os.flush();

        // and get the return value
        char[] grid=new char[Integer.parseInt(br.readLine())];
        for (int i = 0; i < grid.length; i++)
            grid[i] = br.readLine().charAt(0);
        return grid;
    }
    // -----------------------------------------
    void draw() {
        if (!vis) return;
        v.repaint();
    }
    // -----------------------------------------
    public class Vis extends JPanel implements MouseListener, WindowListener {
        public void paint(Graphics g) {
                 
          Color[] colors={Color.BLUE,Color.MAGENTA,Color.YELLOW,Color.RED,Color.CYAN,Color.PINK,Color.GREEN,Color.ORANGE};
              
          int startX=10;
          int startY=10;
          
          // background
          g.setColor(new Color(0xDDDDDD));
          g.fillRect(0,0,N*SZ+230,N*SZ+40);
          g.setColor(Color.WHITE);
          g.fillRect(startX,startY,N*SZ,N*SZ);
          
          //paint thin lines between cells
          g.setColor(Color.BLACK);
          for (int i = 0; i <= N; i++)
            g.drawLine(startX,startY+i*SZ,startX+N*SZ,startY+i*SZ);
          for (int i = 0; i <= N; i++)
            g.drawLine(startX+i*SZ,startY,startX+i*SZ,startY+N*SZ);          
                        

          //paint characters
          g.setFont(new Font("Arial",Font.PLAIN,14));    
          g.setColor(Color.BLACK);        
          
          for (int r = 0; r < N; r++)
            for (int c = 0; c < N; c++)
              if (Board[r][c]==WALL)
              {
                g.setColor(Color.GRAY);
                g.fillRect(startX+c*SZ+1,startY+r*SZ+1,SZ-1,SZ-1);
              }
              else if (isPiece(Board[r][c]))
              {
                for (int m=0; m<names.length; m++)
                  if (Board[r][c]==names[m])   
                  {
                    g.setColor(colors[Players[r][c]]);
                    g.fillRect(startX+c*SZ+1,startY+r*SZ+1,SZ-1,SZ-1);
                    g.drawImage(pics[m],startX+c*SZ,startY+r*SZ,SZ,SZ,null);
                  }
              }
                

              
          //display score and info
          g.setColor(Color.BLACK);                    
          g.setFont(new Font("Arial",Font.BOLD,14));
          g.drawString("SCORE "+Score,SZ*N+25,30);       
          
          //display points
          for (int i=0; i<names.length; i++)
          {
            g.setColor(Color.BLACK);                    
            g.drawString(names2[i]+": "+Points[i]+" points",SZ*N+25,70+20*i);          
          }
                          
          //display player scores
          for (int i=0; i<C; i++)
          {
            g.setColor(colors[i]);  
            g.drawString("Player "+(i+1)+": "+Scores[i],SZ*N+25,200+20*i);
          }
            
            
        }
    
        
        // -------------------------------------
        public Vis()
        {
            jf.addWindowListener(this);             
        }
        // -------------------------------------
        //WindowListener
        public void windowClosing(WindowEvent e){ 
            if(proc != null)
                try { proc.destroy(); } 
                catch (Exception ex) { ex.printStackTrace(); }
            System.exit(0); 
        }
        public void windowActivated(WindowEvent e) { }
        public void windowDeactivated(WindowEvent e) { }
        public void windowOpened(WindowEvent e) { }
        public void windowClosed(WindowEvent e) { }
        public void windowIconified(WindowEvent e) { }
        public void windowDeiconified(WindowEvent e) { }
        // -------------------------------------
        //MouseListener
        public void mouseClicked(MouseEvent e) { }
        public void mousePressed(MouseEvent e) { }
        public void mouseReleased(MouseEvent e) { }
        public void mouseEntered(MouseEvent e) { }
        public void mouseExited(MouseEvent e) { }
    }
    // -----------------------------------------
    Image loadImage(String name) {
/*      try{
        Image im=ImageIO.read(new File(name));
        return im;
      } catch (Exception e) { 
        return null;  
      } */       
      try {
        URL img = MultiplayerChessPiecesVis.class.getResource(name);
        if(img == null)return null;
        Image im = Toolkit.getDefaultToolkit().createImage(img);
        return im;
      } catch (Exception e) { 
        return null;  
      }      
    }    
    // -----------------------------------------
    public MultiplayerChessPiecesVis(String seed) {
      try {
        if (vis)
        {   jf = new JFrame();
            v = new Vis();
            jf.getContentPane().add(v);
            
            pics = new Image[names.length];
            for (int i=0; i<names.length; i++)
              pics[i] = loadImage("images/"+names2[i].toLowerCase()+".png");    //TODO: make sure it is OS agnostic
            
            //TODO: need this?
            MediaTracker mt = new MediaTracker(jf);
            for(int i = 0; i<pics.length; i++){
              if(pics[i] != null)
                mt.addImage(pics[i],i);
            }
            mt.waitForAll();            
        }
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
        SZ = 20;
        Debug = false;
        for (int i = 0; i<args.length; i++)
        {   if (args[i].equals("-seed"))
                seed = args[++i];
            if (args[i].equals("-exec"))
                exec = args[++i];
            if (args[i].equals("-novis"))
                vis = false;
            if (args[i].equals("-size"))
                SZ = Integer.parseInt(args[++i]);
            if (args[i].equals("-debug"))
                Debug = true;
        }
        if (seed.equals("1")) SZ = 50;
            
        MultiplayerChessPiecesVis f = new MultiplayerChessPiecesVis(seed);
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