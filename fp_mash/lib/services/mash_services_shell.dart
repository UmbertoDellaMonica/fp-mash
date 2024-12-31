import 'dart:io';

import 'package:process_run/shell.dart';

class MashShellService {



  // Metodo per mostrare la licenza di mash
  Future<String> showLicense() async {
    final shell = Shell();
    try {
      // Esegui il comando `mash --license`
      final result = await shell.run('mash --license');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from mash --license';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing mash --license: $e';
    }
  }




  // Aggiungi altri metodi per altre operazioni di mash, ad esempio:
  // Esegui Mash command con i parametri
  Future<String> runMashCommand(String command) async {
    final shell = Shell();
    try {
      // Esegui il comando
      final result = await shell.run(command);
      
      // Restituisci l'output del comando
      if (result.isNotEmpty) {
        return result.join('\n');
      } else {
        return 'No output from command: $command';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing command: $command - $e';
    }
  }


  
}
