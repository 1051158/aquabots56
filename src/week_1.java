public class Main {

    public static void main(String[] args) {
        // Dit is een stukje commentaar, dit wordt genegeerd tijdens het uitvoeren van onze code.

        /*
            Dit is commentaar dat geplaatst wordt op meerdere regels.
            Hier kan ik meer informatie plaatsen.
        */

        // Variabelen
        /* Integer */   int number = 5;
        /* Float   */   float decimalNumber = 3.14f;
        /* String  */   String text = "Hello World!";    

        // Informatie tonen in de console (printen)
        System.out.println(text);

        /*
            Bovenstaande code voert een functie uit:
            functienaam(input)
            functienaam: System.out.println
            input: text (aka: "Hello World")
        */

        // Rekenen met Java
        System.out.println(number + " * " + decimalNumber + " = " + (number * decimalNumber));
        System.out.println(number + " - 2 = " + (number - 2));
        System.out.println(number + " / 2 = " + (number / 2));
        System.out.println(number + " + 2 = " + (number + 2));
        System.out.println(number + " % 2 = " + (number % 2));

        // Spelen met tekst
        String hello = "Hello";
        String name = "Wesley";

        System.out.println(hello + " " + name);
        System.out.println("De lengte van de naam is " + name.length());
    }
}
