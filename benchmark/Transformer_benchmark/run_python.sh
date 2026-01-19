cd /home/qc/Chiplet_Heterogeneous_newVersion/benchmark/transformer/python/attention_module
python ./attention_module.py > attention_module.log 2>&1 &
cd ../droppath_module/ 
python ./droppath_module.py > droppath_module.log 2>&1 &
cd ../mlp_module/ 
python ./mlp_module.py > mlp_module.log 2>&1 &
cd ../norm_layer_module/ 
python ./norm_layer_module.py > norm_layer_module.log 2>&1 &