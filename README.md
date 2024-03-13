Γιώργος Μητρόπουλος
sdi2000128

Για μεταγλώττιση poller:
make poller

Για μεταγλώττιση pollSwayer:
make pollSwayer

Περιέχονται τα αρχεία:
Makefile, poller.c, functions_client.c, pollSwayer.c, functions_server.c, types_client.h, types_server.h, create_input.sh, processLogfile.sh, tallyVotes.sh

Για το server κομμάτι της άσκησης χρησημοποιούνται 3 αρχεία:
poller.c, functions_server.c, types_server.h

poller: υπάρχουν κάποιες χρήσιμες μεταβλητές σαν global,
η main ειναι και ο master thread που ζητάει η άσκηση οπου: 
αρχικοποιούνται 3 mutexes και 2 cond variables ώστε να εξασφαλιστεί ο επιθυμητός συγχρονισμός.
Δημιουργείται ενα socket, ενας buffer που κρατάει συνδέσεις που γίνονται μεταξύ client και server.
Ύστερα δημιουργούνται οσα threads (worker threads) δόθηκαν απο την γραμμή εντολών, και σε 1 while loop 
το οποίο επαναλαμβάνεται μέχρι να δοθεί σήμα control c, γίνεται accept σύνδεση με 1 client. και η νέα συνδεση αυτή αποθηκεύται σε ενα μπαφερ.
οσο ο μπαφερ ειναι γεμάτος το master thread περιμένει σε ενα cond variable.

Στο functions_server: υπαρχουν τα worker threads οπου καθε ενα περιμενει σε ενα cond variable αν ο μπαφερ των connections ειναι άδειος, και αν το flag=1 που σημαινει οτι δόθηκε control c τοτε το thread τερματιζει,αλλιώς παίρνει 1 connection απο το buffer και προχωραει στην handle_client οπου εξυπηρετει των πελάτη που εχει ανοίξει συνδεση.

handle_client: στέλνει μήνυμα "SEND NAME PLEASE" και περιμένει απαντηση.
αν ο ψηφοφόρος αυτός εχει ξανα ψηφίσει τοτε στέλνει μήνυμα already voted και κλέινει η σύνδεση, αν οχι τοτε στέλνει μήνυμα "SEND VOTE PLEASE",οπου δημιουργείται νεος voter στον πινακα απο voters που βοηθαει στο να γνωρίζω ποιοι εχουν ψηφισει,(struct), εκτυπόνονται τα κατάλληλα μηνύματα στο log file,και τέλος αν το κόμμα αυτο έχει ξανα ψηφιστεί αυξάνεται κατα 1 οι ψήφοι του αλλιως δημιουργείται νεο κόμμα στον πίνακα απο κομματα. (struct).
Σε μερικά σημεία του κώδικα χρησιμοποιούνται locks , unlocks mutexes για τον σωστο συγχρονισμο (race condition).

όταν δοθει σημα ctrl-c τοτε γίνεται broadcast για ολα τα threads που περιμενουν σε cond var και επειται καλείται η print_close, οπου γίνεται pthread_join για ολα τα thread, εκτύπονονται τα καταλληλα μυνηματα στο αρχειο stats file με την βοήθεια του struct των κομμάτων και τελος γινονται τα απαραίτητα frees.


Για το  client κομμάτι της άσκησης χρησημοποιούνται 3 αρχεία:
pollSwayer.c, functions_client.c, types_client.h

pollSwayer:ανοίγει το file που δόθηκε απο την γραμμή εντολών για διάβασμα και για κάθε γραμμη του:
δημιουργείται ενα object vote (struct) οπου αποθηκευται στο voter_name του το ονομα του ψηφοφόρου και στο party_name το ονομα του κόμματος που ψηφισε.
έπειτα δημιουργείται ενα thread το οποιο μπαίνει σε linked list ωστε να γινουν join στο τέλος ολα μαζι:
functions_client.c:
    send_vote:
δημιουργεί ενα socket για σύνδεση με τον server, συνδέεται, δέχεται μηνύματα και απαντάει αντίστοιχα.Τελος κλέινει την σύνδεση.
Γινονται join μεσω της λιστας ολα τα threads και free.
 
create_input.sh:
ελέγχει αν δόθηκαν σωστά τα ορίσματα στην γραμμή εντολών,
δημιουργει εναν πίνακα parties που περιέχει καθε γραμμή απο το αρχειό politicalPartiesFile,
σε ενα λουπ που τρεχει απο 1 μεχρι των αριθμό των γραμμών που δόθηκαν,
τυπώνει σε κάθε γραμμή του inputFile ενα τυχαίο όνομα και ένα τυχαίο κόμμα απο τον πίνακα parties.

tallyVotes.sh:
ελέγχει αν το inputFile δόθηκε σωστά,
δημιουργεί 2 arrays ,στο ένα θα υπάρχουν τα ονόματα των ψηφοφόρων και στο άλλο οι ψήφοι για κάθε κόμμα.
για κάθε γραμμή απο το inputFile, την χωρίζει σε tokens και στο name= "tokens[0] tokens[1]" ,ελέγχει αν υπάρχει ήδη αυτο το όνομα στο array των ψηφοφόρων, που σημαίνει οτι ο ψηφοφόρος εχει ήδη ψηφίσει, αρα παει στην επόμενη επανάληψη,αλλιώς αυξάνει το partyVotes για το συγκεκριμένο party, τελος εκτυπώνει στο tallyResultsFile το καθε κόμμα μαζι με τις ψήφους του.

processLogFile.sh: ιδια ακριβώς υλοποίηση με το παραπάνω μονο που σε αντίθεση με αυτό που ζητάει η εκφώνηση ( να ελέγχει αν ο ψηφοφόρος εχει ήδη ψηφίσει). αυτό δεν ελέγχεται καθώς δεν χρειάζεται αφού το poll-log file ,που έχει δημιουργηθεί απο το poller, δεν εχει μέσα διπλότυπα.



