import 'dart:io';
import 'package:process_run/shell.dart';

class DskShellService {
  final Shell shell = Shell();

  // Metodo per eseguire 'dsk -help'
  Future<String> showHelp() async {
    try {
      // Esegui il comando `dsk -help`
      final result = await shell.run('dsk -help');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        //result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from dsk -help';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing dsk -help: $e';
    }
  }

  Future<String> generateH5File(
      String inputFilePath, String outputDirectory) async {
    try {
      final result = await shell.run('dsk -file $inputFilePath -out-dir $outputDirectory');

      // Controlla se il comando è stato eseguito correttamente
      if (result.isNotEmpty == 0) {
        //result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text; // Restituisci l'output del comando
      } else {
        return 'Error executing dsk ';
      }
    } catch (e) {
      return 'Exception: $e'; // Restituisci l'errore in caso di eccezione
    }
  }
}
