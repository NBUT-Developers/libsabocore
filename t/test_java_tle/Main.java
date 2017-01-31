import java.util.Scanner;
public class Main
{
    public static void main(String[] args)
    {
        Scanner scan = new Scanner (System.in);
        int a, b;
        while (scan.hasNextInt())
        {
            a = scan.nextInt();
            b = scan.nextInt();
            System.out.println(a + b);
        }
        while (true);
    }
}
