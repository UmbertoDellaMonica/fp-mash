import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:fp_mash/services/directory_service.dart';

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {



  final DirectoryService _directoryService = DirectoryService(); // Istanza del servizio

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: const Icon(Icons.science), // Aggiungi l'icona qui
        title: Text(widget.title), // Usa il titolo fornito dal widget
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(Icons.biotech_sharp, size: 50.0),
                SizedBox(width: 10),
                Text(
                  'fp-mash',
                  style: TextStyle(fontSize: 40.0, fontWeight: FontWeight.bold),
                ),
                SizedBox(width: 10),
                Icon(Icons.biotech, size: 50.0),
              ],
            ),
            const SizedBox(height: 20),
            const Padding(
              padding: EdgeInsets.symmetric(horizontal: 20.0),
              child: Text(
                'This software generates fingerprints on genetic sequences and computes the distance between two considered fingerprints.',
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 18.0),
              ),
            ),
            const SizedBox(height: 40),
            ElevatedButton(
              onPressed: () async {
                // Crea la directory per il primo step
                await _directoryService.createStepDirectory(_directoryService.step1Directory);
                
                // Naviga al prossimo step
                Navigator.pushNamed(context, '/step1');
              },
              child: const Text('Start'),
            ),
          ],
        ),
      ),
    );
  }
}

