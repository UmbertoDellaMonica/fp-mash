import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}



class _MyHomePageState extends State<MyHomePage> {
  @override
  Widget build(BuildContext context) {


    return Scaffold(
      /// AppBar 
      appBar: AppBar(
        leading: Icon(Icons.science), // Aggiungi l'icona qui
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
              onPressed: () {
                /// Navigate to the next step in the pipeline
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
