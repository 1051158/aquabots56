public class Main {
    public static void main(String[] args) {
        //uncomment de functies voor de weekopdrachten voor de bijbehorende week
        //Week_1(args);
        berekeningen(args);

    }
    public static void berekeningen(String[] args)
    {
        int T;
        float Rpar[] = {46768,44669,42176,39452,36391,33242,29965,26704,23548,20572,17830,15361,13162};
        T = -30;
        float Kpar;
        float Vo;
        float Vref = 5;
        float R;
        for(int i = 0; i <= 12; i++)
        {
            Kpar = (Rpar[i]-Rpar[6])/(Rpar[6] * T);
            R = (Rpar[6] + (Rpar[6] * Kpar * T));
            System.out.print("Kpar: ");
            System.out.println(Kpar);
            System.out.println("R = " + R);
            Vo = Vref*Kpar*T/2;
            System.out.println("Vo + " + Vo);
            T+= 5;
        }
    }
}
    /*public static void Week_1(String[] args)
    {
        //in deze functie staan alle opdrachten van week 1
        //Opdracht 1
        String naam = "Donny";
        System.out.println("Hallo, " + naam + " Welkom");

        //Opdracht 2
        int getal_1 = 10;
        int getal_2 = 20;
        int getal_3 = getal_1 + getal_2;
        System.out.println(getal_3);

        //Opdracht 3
        //naam van opdracht 1 is gebruikt hier!
        int leeftijd = 25;
        System.out.println(naam + " is " + leeftijd + " jaar oud");

        //Opdracht 4
        float kommagetal = 5.67f;
        float vermenigvuldiging = kommagetal * 2;
        System.out.println("het dubbele van " + kommagetal + " is " + vermenigvuldiging);

        //Opdracht 5
        float deling = kommagetal/2;
        System.out.println("de helft van " + kommagetal + " is " + deling);

        //Opdracht 6
        String text_1 = "dit is de eerste stuk tekst ";
        String text_2 = "en dit is het 2e stuk tekst";
        System.out.println(text_1 + text_2);

        //Opdracht 7
        float getal_4 = 5.67f;
        float getal_5 = 2.33f;
        System.out.println("som: " + getal_4 + getal_5);
        System.out.print("Verschil: ");
        System.out.println(getal_4 - getal_5);
        System.out.print("vermenigvuldiging: ");
        System.out.println(getal_4 * getal_5);
        System.out.println(getal_4 / getal_5);

        //Opdracht 8
        //naam van Opdracht 1 is gebruikt hier!
        for(int i = 1; i <= 5; i++)
        {
            System.out.println(i + ": " + naam);
        }

        //Opdracht 9
        char S_getal = '8';
        String S_getal_2 = "10";
        System.out.println(S_getal + S_getal_2);

        //Opdracht 10
        //voor naam 1 is de naam uit opdracht 1 gebruikt
        String naam_2 = "Dennis";
        int verschil = naam.length() - naam_2.length();
        System.out.println("het verschil tussen" + naam + "en" + naam_2 + "is: " + verschil);

    }
    
    public static void Week_3(String[] args)
    {
        //Opdracht 1
        int a[] = {5,7};
        int biggest = Grootste(a[0], a[1]);
        if(biggest == 0)
            System.out.println("getallen zijn even groot");
        
        if(biggest != 0)
            System.out.println(biggest);
        
        //Opdracht 2
        int length = strlen("wat is hier de lengte van?");
        System.out.println(length);

        //Opdracht 3
        int som = Som(9,6);
        System.out.println(som);

        //Opdracht 4 (print Donny 6x met enter erbij toegevoegd)
        String total_tekst = rePrint("Donny\n", 6);
        System.out.println(total_tekst);
    }
    //Opdracht 3.2
    public static int Grootste(int getal_1, int getal_2)
    {
        int grootste;
        if(getal_1 > getal_2)
            {
                grootste = getal_1;
                return grootste;
            }
        if(getal_1 < getal_2)
        {
            grootste = getal_2;
            return grootste;
        }
        grootste = 0;
        return grootste;
    }
    //Opdracht 3.2
    public static int strlen(String tekst)
    {
        int lengte = tekst.length();
        return lengte;
    }

    //Opdracht 3.3
    public static int Som(int getal_1, int getal_2)
    {
        int optelling = getal_1 + getal_2;
        return optelling;
    }
    //Opdracht 3.4
    public static String rePrint(String tekst, int aantal)
    {
        String total_tekst = "";
        for(int i = 0; i < aantal; i++)
        {
            total_tekst = total_tekst + tekst;
        }
        return total_tekst;
    }
}
*/
